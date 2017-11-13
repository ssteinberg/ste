
#include <common.glsl>
#include <voxels.glsl>


layout(set=1, binding=2) uniform usampler2D voxels;
layout(set=1, binding=3) uniform sampler3D bricks_albedo;		// RGBA8 unorm
layout(set=1, binding=4) uniform sampler3D bricks_roughness;	// R8G8 unorm
layout(set=1, binding=5) uniform sampler3D bricks_metadata;		// RGBA8 unorm


struct voxel_traversal_result_t {
	float distance;
	float accumulated_vacancy;

	voxel_unpacked_data_t data;
};

uint voxels_read(uint ptr) {
	return texelFetch(voxels, voxels_image_coords(ptr), 0).x;
}


voxel_unpacked_data_t voxel_read_and_decode_brick(vec3 v, ivec3 b, uint brick_ptr, int level, const bvec3 b_dir_lt_zero) {
	const float grid_res = float(voxel_resolution(level - 1));
	const vec3 uvw = mod(v * grid_res, 2.f);
	const vec3 coord = voxels_brick_texture_coords(brick_ptr, uvw);

	// Read and interpolate bricks
	vec2 roughness_occupancy = textureLod(bricks_roughness, coord, 0).xy;
	float normalizer = 1.f / roughness_occupancy.y;

	vec4 meta = textureLod(bricks_metadata, coord, 0) * normalizer;
	roughness_occupancy.x *= normalizer;

	voxel_unpacked_data_t ret;
	ret.albedo = textureLod(bricks_albedo, coord, 0) * normalizer;

	ret.normal = snorm2x32_to_norm3x32(fma(meta.xy, vec2(2.f), vec2(-1.f)));
	ret.ior = mix(material_layer_min_ior, material_layer_max_ior, meta.z);
	ret.metallicity = meta.w;
	ret.roughness = roughness_occupancy.x * voxel_data_roughness_max_value;

	return ret;
}

/**
*	@brief	Decodes data out of a lead node
*/
voxel_unpacked_data_t voxel_read_and_decode_leaf_node(uint node) {
	voxel_data_t data;
	data.packed[0] = voxels_read(node + 0);
	data.packed[1] = voxels_read(node + 1);
	data.packed[2] = voxels_read(node + 2);

	voxel_unpacked_data_t ret;
	decode_voxel_data(data, 
					  ret.albedo.rgb,
					  ret.normal,
					  ret.roughness,
					  ret.albedo.a,
					  ret.ior,
					  ret.metallicity);

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
	ivec3 b;
 
	// Init
	uint stack[voxel_leaf_level - 1];
	uint node = voxel_root_node;
	uint brick_ptr;
	int level = 0;

	// Traverse
	const bool no_step_limit = step_limit == 0xFFFFFFFF;
	uint i=0;
	for (; no_step_limit || i<step_limit; ++i) {
		const int level_resolution = int(P) * (n - level);

		// Calculate brick coordinates
		b = (u >> level_resolution) & int(voxel_mask);
		const uint brick_idx = voxel_brick_index(b) ^ a;
 
		// Check if we have child
		const uint occupancy_map_ptr = node + voxel_node_binary_map_offset();
		const bool has_child = (voxels_read(occupancy_map_ptr) & (1 << brick_idx)) != 0;
		if (has_child) {
			// Step in
			++level;
   
			if (level == voxel_leaf_level) {
				// Hit leaf
				brick_ptr = voxels_read(node + voxel_node_brick_image_address_offset());
				break;
			}
			
			node = voxels_read(node + voxel_node_children_offset() + brick_idx);
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

	if (!no_step_limit && i == step_limit)
		level = -1;
 
	voxel_traversal_result_t ret;
	if (level > 0) {
		// Realign
		v = mix(v, 1.f - v, b_dir_lt_zero);
		b = mix(b, 1 - b, b_dir_lt_zero);

		// Compute distance
		const vec3 pos = (v - .5f) * voxel_world;
		ret.distance = length(pos - V);
 
		// Read and interpolate data for ray traces
		ret.data = voxel_read_and_decode_brick(v, b, brick_ptr, level, b_dir_lt_zero);
		//ret.data = voxel_read_and_decode_leaf_node(node);
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
