
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
	const float delta = .25f * voxel_grid_resolution / voxel_world;

	vec3 sign_dir = sign(dir);
	vec3 edge = mix(vec3(.0f), vec3(1.f), greaterThan(dir, vec3(.0f)));
	vec3 recp_dir = 1.f / dir;

	// Compute voxel world positions
	vec3 v = V / voxel_world + .5f;

	// Init stack
	uint stack[voxel_leaf_level];
	stack[0] = voxel_root_node;

	// Traverse
	uint node = stack[0];
	vec3 u = v * float(1 << voxel_Pi);
	uint P = voxel_Pi;

	int level = 0;
	while (true) {
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
			if (level == voxel_leaf_level) {
				// Hit
				vec3 pos = (v - .5f) * voxel_world;
				
				voxel_traversal_result_t ret;
				ret.hit_voxel = node;
				ret.distance = length(pos - V);

				return ret;
			}

			// Read child node address
			uint child_ptr = node + voxel_node_children_offset(P) + brick_idx;

			// Read new level parameters
			P = voxel_P;
			node = voxel_buffer[child_ptr];
			u = brick_coord * float(1 << P);

			// Update stack
			stack[level] = node;

			continue;
		}

		// No child, traverse.
		vec3 t = recp_dir * sign_dir * max(sign_dir * (edge - brick_coord), vec3(1e-4f)); 		// Avoid nan created by division of 0 by inf
		float t_bar = min_element(t) + delta;
		
		uint prev_level = level;
		float level_resolution = float(voxel_resolution(level));
		vec3 step = t_bar * dir;
		u += step;

		while (any(lessThan(u, vec3(.0f))) || any(greaterThanEqual(u, vec3(1 << P)))) {
			// Step out
			--level;
			if (level < 0) {
				// No voxel was hit
				voxel_traversal_result_t ret;
				ret.distance = +inf;
				return ret;
			}
			
			// Pop stack
			node = stack[level];
			P = voxel_block_power(level);

			// Coordinates at level's block
			u = v * float(voxel_resolution(level));
			u = mod(u, float(1 << P));
			u += step / float(voxel_resolution_difference(level, prev_level));
		}
			
		v += step / level_resolution;
	}
}
