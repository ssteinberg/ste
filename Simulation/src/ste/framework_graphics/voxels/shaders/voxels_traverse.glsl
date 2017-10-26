
#include <common.glsl>
#include <voxels.glsl>


layout(set=1, binding = 0) uniform usampler2D voxels;


struct voxel_unpacked_data_t {
	vec3 normal;
	float roughness;
	vec3 albedo;
	float opacity;
};

struct voxel_traversal_result_t {
	float distance;
	voxel_unpacked_data_t data;
};

uint voxels_read(uint ptr) {
	uint x = ptr % voxel_buffer_line;
	uint y = ptr / voxel_buffer_line;
	return texelFetch(voxels, ivec2(x,y), 0).x;
}

/**
*	@brief	Traverses a ray from V in direction dir, returning the first hit voxel, if any.
*/
voxel_traversal_result_t voxel_traverse(vec3 V, vec3 dir, uint step_limit) {
	const bvec3 b_dir_gt_z = greaterThan(dir, vec3(.0f));
	const vec3 edge = mix(vec3(.0f), vec3(1.f), b_dir_gt_z);
	const vec3 recp_dir = 1.f / dir;
	const vec3 sign_dir = sign_ge_z(dir);

	const uint n = voxel_leaf_level - 1;
	const float grid_res = float(voxel_resolution(n));
	const float grid_res_0 = float(voxel_resolution(0));
	const vec2 res_initial = vec2(grid_res_0, 1.f / grid_res_0);
	const vec2 res_step = vec2(float(1 << voxel_P), 1.f / float(1 << voxel_P));

	const int full_mask = 0xFFFFFFFF;
	const int intermediate_levels_msb = int(voxel_P * n) - 1;
	const ivec2 Pi_P_vec = ivec2(voxel_Pi, voxel_P);

	// Compute voxel world positions
	vec3 v = V / voxel_world + .5f;
	if (any(lessThan(v, vec3(0))) || any(greaterThanEqual(v, vec3(1)))) {
		// Outside voxel volume
		const vec3 world_edge = mix(vec3(1.f), vec3(.0f), b_dir_gt_z);
		const vec3 t = recp_dir * sign_dir * max((world_edge - v) * sign_dir, vec3(1e-30f)); 		// Avoid nan created by division of 0 by inf
		const float t_bar = min_element(t);
		v += dir * t_bar;

		if (any(lessThan(v, vec3(0))) || any(greaterThanEqual(v, vec3(1)))) {
			// No voxel can be hit 
			voxel_traversal_result_t ret;
			ret.distance = +inf;
			return ret;
		}
	}

	// Init
	uint node = voxel_root_node;
	uint P = voxel_Pi;
	int level_resolution = int(voxel_P * n);
	vec2 res = res_initial;

	ivec3 u = ivec3(v * grid_res);
	ivec3 b = (u >> level_resolution) % int(1 << voxel_Pi);

	// Traverse
	int level = 0;
	for (uint i=0; i<step_limit; ++i) {
		// Calculate brick coordinates
		const uint brick_idx = voxel_brick_index(b, P);

		// Read child node address
		const uint child_ptr = node + voxel_node_children_offset(level, P) + brick_idx;
		const uint child = voxels_read(child_ptr);

		// Check if we have child here
		const bool has_child = child != 0;
		if (has_child) {
			// Step in
			++level;
			node = child;
			
			if (level == voxel_leaf_level) {
				// Hit leaf
				break;
			}

			// Read new level parameters
			level_resolution -= int(voxel_P);
			res *= res_step;
			P = voxel_P;

			b = (u >> level_resolution) % int(1 << voxel_P);

			continue;
		}

		// No child, traverse.
		const vec3 f = fma(v, res.xxx, -vec3(u >> level_resolution));
		const vec3 t = recp_dir * sign_dir * max((edge - f) * sign_dir, vec3(1e-30f)); 		// Avoid nan created by division of 0 by inf
		const float t_bar = min_element(t);
		const bvec3 mixer = equal(vec3(t_bar), t);
		const vec3 step = max(t_bar, 1e-3f) * dir;

		// Step v
		v += step * res.y;

		const ivec3 u_hat = bitfieldInsert(u, full_mask.xxx, 0, level_resolution) + mix(-ivec3(1 << level_resolution), ivec3(1), b_dir_gt_z);
		const ivec3 u_bar = mix(ivec3(v * grid_res), u_hat, mixer);
		
		const ivec3 b_offset = (u_bar >> level_resolution) - (u >> level_resolution); 
        ivec3 b_bar = b + b_offset; 

		// Have we moved past the block edge?
		const bool past_edge = any(lessThan(b_bar, ivec3(0))) || any(greaterThanEqual(b_bar, ivec3(1 << P)));
		if (past_edge) {
			if (level == 0) {
				// No voxel was hit 
				break;
			}

			// Restart
			level = 0;
			node = voxel_root_node;
			P = voxel_block_power(level); 
			level_resolution = int(voxel_P * n);
			res = res_initial; 

			const ivec3 u_shift = u >> level_resolution; 
			const ivec3 b_offset2 = (u_bar >> level_resolution) - u_shift; 
			b_bar = u_shift % int(1 << P) + b_offset2;
		}
		
		b = b_bar;
		u = u_bar; 
	}

	
	voxel_traversal_result_t ret;

	if (level == voxel_leaf_level) {
		// Compute distance
		const vec3 pos = (v - .5f) * voxel_world;
		ret.distance = length(pos - V);
		
		// Read data
		uint data_ptr = node + voxel_node_data_offset(level, voxel_P);
		voxel_data_t data;
		data.albedo_packed = voxels_read(data_ptr + 0);
		data.normal_packed = voxels_read(data_ptr + 1);
		data.opacity_roughness_packed = voxels_read(data_ptr + 2);
		decode_voxel_data(data, ret.data.normal, ret.data.roughness, ret.data.albedo, ret.data.opacity);
	}
	else {
		// No hit
		ret.distance = +inf;
	}
				
	return ret;
}
