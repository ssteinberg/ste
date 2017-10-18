
#include <common.glsl>
#include <voxels.glsl>

struct voxel_traversal_result_t {
	float distance;
	uint hit_voxel;
};

voxel_traversal_result_t voxel_traverse(vec3 V, vec3 dir) {
	// Traversal limits
	const vec3 scene_min = vec3(.0f);
	const vec3 scene_max = vec3(1.f);
	const float delta = .5f * voxel_grid_resolution / voxel_world;
	
	vec3 edge = mix(vec3(.0f), vec3(1.f), greaterThan(dir, vec3(.0f)));
	vec3 recp_dir = 1.f / dir;
	
	// Compute voxel world positions
	vec3 v = V / voxel_world + .5f;

	for (int i=0;i<1000 && all(lessThan(v, scene_max)) && all(greaterThanEqual(v, scene_min));++i) {
		uint node = voxel_root_node;
		int level = 0;

		float block = voxel_block_extent(level);
		uint P = voxel_block_power(level);

		float step_size = 1.f / block;
		vec3 u = v * block;

		while (level < voxel_leaf_level) {
			// Calculate brick coordinates
			vec3 brick_coord = fract(u);
			uint brick_idx = voxel_brick_index(uvec3(u), P);

			// Compute binary map and pointer addresses
			const uvec2 binary_map_address = voxel_binary_map_address(brick_idx);
			const uint binary_map_word_ptr = node + voxel_node_binary_map_offset(P) + binary_map_address.x;

			// Check if we have child here
			bool has_child = ((voxel_buffer[binary_map_word_ptr] >> binary_map_address.y) & 0x1) == 1;
			if (has_child) {
				// Step in
				++level;
				
				uint child_ptr = node + voxel_node_children_offset(P) + brick_idx;
				node = voxel_buffer[child_ptr];
				
				block = voxel_block_extent(level);
				P = voxel_block_power(level);

				step_size /= block;
				u = brick_coord * block;

				continue;
			}

			// No child, traverse.
			vec3 t = (edge - brick_coord) * recp_dir;
			float xi = max(t.x, max(t.y, t.z));
			float t0 = mix(t.x, t.y, .5f);
			float t1 = mix(t.x, t.z, .5f);
			float t2 = mix(t.z, t.y, .5f);
			float t_bar = mix(t0, mix(t1, t2, xi == t.x), xi != t.z);
			
			t_bar = min_element(t) + delta;

			vec3 step = t_bar * dir;
			u += step;
			v += step * step_size;

			if (any(lessThan(u, vec3(.0f))) || any(greaterThanEqual(u, vec3(block)))) {
				// Restart
				break;
			}
		}
		
		// Hit a voxel?
		if (level == voxel_leaf_level) {
			vec3 P = (v - .5f) * voxel_world;
			
			voxel_traversal_result_t ret;
			ret.hit_voxel = node;
			ret.distance = length(P - V);

			return ret;
		}
	}
	
	// No voxel was hit
	voxel_traversal_result_t ret;
	ret.distance = +inf;

	return ret;
}
