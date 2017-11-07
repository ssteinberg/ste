
#include <common.glsl>
#include <voxels.glsl>


layout(set=1, binding = 2) uniform usampler2D voxels;
layout(set=1, binding=3) uniform sampler3D bricks_albedo;		// RGBA8 unorm
layout(set=1, binding=4) uniform sampler3D bricks_normal;		// RGBA8 unorm
layout(set=1, binding=5) uniform sampler3D bricks_metadata;		// RGBA8 unorm


struct voxel_unpacked_data_t {
	vec3 normal;
	vec3 albedo;
	vec4 roughness_opacity_ior_metallicity;
};

struct voxel_traversal_result_t {
	float distance;
	float accumulated_vacancy;

	voxel_unpacked_data_t data;
};

uint voxels_read(uint ptr) {
	uint x = ptr & (voxel_buffer_line-1);
	uint y = ptr / voxel_buffer_line;
	return texelFetch(voxels, ivec2(x,y), 0).x;
}

voxel_unpacked_data_t mix(voxel_unpacked_data_t x, voxel_unpacked_data_t y, float a) {
	voxel_unpacked_data_t r;
	r.normal = mix(x.normal, y.normal, a);
	r.albedo = mix(x.albedo, y.albedo, a);
	r.roughness_opacity_ior_metallicity = mix(x.roughness_opacity_ior_metallicity, y.roughness_opacity_ior_metallicity, a);

	return r;
}

voxel_unpacked_data_t voxel_read_and_decode_data(uint node, uint level) {
	voxel_data_t data;

	uint data_ptr = node + voxel_node_data_offset(level);
	data.packed[0] = voxels_read(data_ptr + 0);
	data.packed[1] = voxels_read(data_ptr + 1);
	data.packed[2] = voxels_read(data_ptr + 2);
	data.packed[3] = voxels_read(data_ptr + 3);

	voxel_unpacked_data_t ret;
	decode_voxel_data(data, 
					  ret.albedo, 
					  ret.normal, 
					  ret.roughness_opacity_ior_metallicity.x, 
					  ret.roughness_opacity_ior_metallicity.y, 
					  ret.roughness_opacity_ior_metallicity.z, 
					  ret.roughness_opacity_ior_metallicity.w);

	return ret;
}


/**
* @brief Traverses a ray from V in direction dir, returning the first hit voxel, if any.
*/
voxel_traversal_result_t voxel_traverse(vec3 V, vec3 D, uint step_limit, float step_length) {
	const bvec3 b_dir_lt_zero = lessThan(D, vec3(.0f));
	const vec3 dir = max(abs(D), vec3(1e-10f));
	const vec3 recp_dir = 1.f / dir;
 
	const uvec3 invert_dir = ivec3(b_dir_lt_zero) * uvec3(4,2,1);
    const uint a = invert_dir.x + invert_dir.y + invert_dir.z;

	const uint P = voxel_P;
	const int n = int(voxel_leaf_level) - 1;
	const float grid_res = float(voxel_resolution(n));
	
	// Compute voxel world positions
	vec3 v = V / voxel_world + .5f;
	v = mix(v, 1.f - v, b_dir_lt_zero);
	ivec3 u = ivec3(v * grid_res);
 
	// Init
	uint stack[voxel_leaf_level - 1];
	uint node = voxel_root_node;
	int level = 0;
 
	// Traverse
	const bool no_step_limit = step_limit == 0xFFFFFFFF;
	for (uint i=0; no_step_limit || i<step_limit; ++i) {
		const int level_resolution = int(P * (n - level));

		// Calculate brick coordinates
		ivec3 b = (u >> level_resolution) & ((1 << P) - 1);
		const uint brick_idx = voxel_brick_index(b) ^ a;
 
		// Read child node address
		const uint child_ptr = node + voxel_node_children_offset(level) + brick_idx;
		const uint child = voxels_read(child_ptr);
 
		// Check if we have child
		if (child != 0) {
			// Step in
			++level;
			node = child;
   
			if (level == voxel_leaf_level) {
				// Hit leaf
				break;
			}
			
			stack[level - 1] = child;
		}
		else {
			const float res = float(voxel_resolution(level));
			const float recp_res = 1.f / res;

			// No child, traverse.
			const vec3 f = fma(-v, res.xxx, vec3(u >> level_resolution));
			const vec3 t = fma(recp_dir, f, recp_dir);
			const float t_bar = step_length == .0f ? min_element(t) : step_length;
			const bvec3 mixer = equal(vec3(t_bar), t);
			const vec3 step = max(t_bar, 1e-3f) * dir;
 
			// Step v
			v = fma(step, recp_res.xxx, v);
 
			const vec3 u_hat = mix(vec3(0), vec3(0.5), mixer);
			const ivec3 u_bar = step_length == .0f ? ivec3(fma(v, grid_res.xxx, u_hat)) : ivec3(v * grid_res);
  
			// Pop, if needed
			const int msb = max_element(findMSB(u_bar ^ u));
			level = n - msb;
			if (level < 0) {
				// No voxel was hit 
				break;
			}
  
			node = level == 0 ? voxel_root_node : stack[level-1];
			u = u_bar; 
		}
	}
 
 
	voxel_traversal_result_t ret;
	if (level > 0) {
		// Compute distance
		v = mix(v, 1.f - v, b_dir_lt_zero);
		const vec3 pos = (v - .5f) * voxel_world;
		ret.distance = length(pos - V);
 
		// Read and interpolate data for ray traces
		ret.data = voxel_read_and_decode_data(node, level);
	}
	else {
		// No hit
		ret.distance = +inf;
	}
    
	return ret;
}

/**
*	@brief	Traverses a ray from V in direction dir, returning the first hit voxel, if any.
*/
voxel_traversal_result_t voxel_traverse_ray(vec3 V, vec3 dir) {
	return voxel_traverse(V, 
						  dir, 
						  0xFFFFFFFF, 
						  .0f);
}

/**
*	@brief	Traverses a ray from V in direction dir, returning the first hit voxel, if any.
*/
voxel_traversal_result_t voxel_traverse_ray(vec3 V, vec3 dir, uint step_limit) {
	return voxel_traverse(V, 
						  dir, 
						  step_limit, 
						  .0f);
}

/**
*	@brief	Traverses a ray from V in direction dir, returning the first hit voxel, if any.
*/
voxel_traversal_result_t voxel_traverse_ray_fixed_step(vec3 V, vec3 dir, float step_length, uint step_limit) {
	return voxel_traverse(V, 
						  dir, 
						  step_limit, 
						  step_length);
}
