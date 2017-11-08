
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
	return texelFetch(voxels, voxels_image_coords(ptr), 0).x;
}


voxel_unpacked_data_t voxel_read_and_decode_data(vec3 v, uint brick_ptr, int level, const bvec3 b_dir_lt_zero) {
	const float grid_res = float(voxel_resolution(level));
	vec3 f = fract(v * grid_res);
	f = mix(f, 1.f - f, b_dir_lt_zero);

	const ivec3 brick_idx = ivec3(round(f * 2.f));
	
	const ivec3 coord = voxels_brick_image_coords(brick_ptr) + brick_idx;

	voxel_unpacked_data_t ret;
	ret.albedo = texelFetch(bricks_albedo, coord, 0).rgb;

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
		const int level_resolution = int(P) * (n - level);

		// Calculate brick coordinates
		const ivec3 b = (u >> level_resolution) & ((1 << P) - 1);
		const uint brick_idx = voxel_brick_index(b) ^ a;
 
		// Check if we have child
		const uint occupancy_map_ptr = node + voxel_node_binary_map_offset();
		const bool has_child = (voxels_read(occupancy_map_ptr) & (1 << brick_idx)) != 0;
		if (has_child) {
			// Step in
			++level;

			// Read child node address
			const uint children_address = node + voxel_node_children_offset();
			
   
			if (level == voxel_leaf_level) {
				// Hit leaf
				node = voxels_read(children_address);
				break;
			}
			
			node = voxels_read(children_address + brick_idx);
			stack[level - 1] = node;
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
 
			node = level == 0 ? voxel_root_node : stack[level - 1];
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
		ret.data = voxel_read_and_decode_data(v, node, level, b_dir_lt_zero);
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
