
#include <voxels.glsl>

uint voxel_voxelize_traverse_tree_node(uint node,
									   uint level,
									   uint P,
									   uint brick_idx) {
	uint child_level = level + 1;

	// Compute binary map and pointer addresses
	const uint child_ptr = node + voxel_node_children_offset(P) + brick_idx;
	const uvec2 binary_map_address = voxel_binary_map_address(brick_idx);
	const uint binary_map_word_ptr = node + binary_map_address.x;

	// Check if the brick is already populated
	bool populated = (voxel_buffer[binary_map_word_ptr] & (1 << binary_map_address.y)) != 0;
	// If populated, read child offset.
	uint child_node = voxel_buffer[child_ptr];
	if (!populated) {
		uint semaphore_pointer = node + voxel_node_semaphore_offset(P);

		// Create child pointer
		bool done = false;
		while (!done) {
			// Lock
			bool locked = atomicExchange(voxel_buffer[semaphore_pointer], voxel_lock_pattern) == 0;
			if (locked) {
				// We got lock
				
				uint occupancy_word = voxel_buffer[binary_map_word_ptr];
				if ((occupancy_word & (1 << binary_map_address.y)) == 0) {
					// Allocate memory for child.
					uint child_size = voxel_node_size(child_level);

					// Write child and occupancy bit
					voxel_buffer[child_ptr] = atomicAdd(voxel_buffer_size, child_size);
					voxel_buffer[binary_map_word_ptr] = occupancy_word | (1 << binary_map_address.y);
					
					memoryBarrierBuffer();
				}
		
				// Release lock.
				atomicExchange(voxel_buffer[semaphore_pointer], 0);
				child_node = voxel_buffer[child_ptr];
				
				done = true;
			}
		}
	}

	// TODO: Append user data

	return child_node;
}

void voxelize(vec3 V) {
	// Compute voxel world coordinate (assumes V is inside voxel world)
	vec3 v = clamp(V / voxel_world + .5f, vec3(.0f), vec3(1.f) - .25f * voxel_grid_resolution / voxel_world);

	// Traverse tree till leaf, creating nodes on the way, if necessary.
	uint node = voxel_root_node;
	for (uint level=0; level<voxel_leaf_level; ++level) {
		float block = voxel_block_extent(level);
		uint P = voxel_block_power(level);
		
		// Calculate brick coordinates and index in block
		vec3 brick = v * block;
		uint brick_idx = voxel_brick_index(uvec3(brick), P);

		// Traverse tree (will generate a node, if needed)
		node = voxel_voxelize_traverse_tree_node(node, 
												 level, 
												 P,
												 brick_idx);

		// Calculate coordinates in brick
		v = fract(brick);
	}
}
