
// (2^P)^3 voxels per block
layout(constant_id=5) const uint voxel_P = 2;
// (2^Pi)^3 voxels per initial block
layout(constant_id=6) const uint voxel_Pi = 3;
// Voxel structure end level index
layout(constant_id=7) const uint voxel_leaf_level = 5;

// Voxel world extent
layout(constant_id=8) const float voxel_world = 1000;


const uint voxel_lock_pattern = 0xFFFFFFFF;


layout(std430, set=0, binding=0) restrict buffer voxel_buffer_binding {
	uint voxel_buffer[];
};
layout(std430, set=0, binding=1) restrict buffer voxel_counter_binding {
	uint voxel_buffer_size;
};


#include <voxels_utils.glsl>


uint voxel_traverse_tree_node(uint offset,
							  uint level,
							  uint P,
							  uint brick_idx) {
	uint child_level = level + 1;

	// Compute binary map and pointer addresses
	const uvec2 binary_map_address = voxel_binary_map_address(brick_idx);
	const uint binary_map_word_ptr = offset + binary_map_address.x;
	const uint child_ptr = offset + voxel_node_children_offset(P) + brick_idx;

	// Check if the brick is already populated
	bool populated = ((voxel_buffer[binary_map_word_ptr] >> binary_map_address.y) & 0x1) == 1;
	// If populated, read child offset.
	uint child_offset;
	if (populated) {
		// Atomically load child pointer
		child_offset = atomicCompSwap(voxel_buffer[child_ptr], 0, 0);
	}
	else {
		// Atomically create/read child pointer
		bool done = false;
		while (!done) {
			// Lock
			child_offset = atomicCompSwap(voxel_buffer[child_ptr], 0, voxel_lock_pattern);
			if (child_offset == 0) {
				// We got lock
				// We would like to release the lock as soon as possible. Setting the binary map bit will allow subsequent 
				// threads to avoid the expensive atomicCompSwap, but it can be done after writing the child pointer. 
				// The rest of the metadata isn't used during the voxelization process.

				// Allocate memory for child.
				uint child_size = voxel_node_size(child_level);
				child_offset = atomicAdd(voxel_buffer_size, child_size);
		
				// Write child address, releasing lock.
				atomicExchange(voxel_buffer[child_ptr], child_offset);
				memoryBarrierBuffer();

				// Set binary map bit
				uint binary_map_word = voxel_buffer[binary_map_word_ptr];
				uint binary_map_word_new_value;
				while ((binary_map_word_new_value = atomicCompSwap(voxel_buffer[binary_map_word_ptr], 
																   binary_map_word, 
																   binary_map_word | (1 << binary_map_address.y))) != binary_map_word) {
					binary_map_word = binary_map_word_new_value;
				}

				// Set occupancy bit
				uint parent_ptr = offset + voxel_node_parent_offset(P);
				voxel_buffer[parent_ptr] |= 0x80000000;

				// Set child parent (for non-leaves)
				if (child_level != voxel_leaf_level) {
					uint child_parent_ptr = child_offset + voxel_node_parent_offset(voxel_block_power(child_level));
					voxel_buffer[child_parent_ptr] = offset;
				}

				memoryBarrierBuffer();
			}

			if (child_offset != voxel_lock_pattern)
				done = true;
		}
	}

	// TODO: Append user data

	return child_offset;
}

void voxelize(vec3 V) {
	// Compute voxel world coordinate (assumes V is inside voxel world)
	vec3 v = clamp(V / voxel_world + .5f, vec3(.0f), vec3(1.f) - .1f / voxel_grid_resolution);

	// Traverse tree till leaf, creating nodes on the way, if necessary.
	uint pointer = 0;
	for (uint level=0; level<voxel_leaf_level; ++level) {
		vec3 block = voxel_block_extent(level);
		uint P = voxel_block_power(level);
		
		// Calculate brick coordinates and index in block
		vec3 brick = v * block;
		uint brick_idx = voxel_brick_index(uvec3(brick), P);

		// Traverse tree (will generate a node, if needed)
		pointer = voxel_traverse_tree_node(pointer, 
										   level, 
										   P,
										   brick_idx);

		// Calculate coordinates in brick
		v = fract(brick);
	}
}
