
#include <voxels.glsl>

/**
*	@brief	Traverses a node, atomically creating a child at supplied position.
*/
void voxel_voxelize(inout float x, 
					inout float y, 
					inout float z, 
					inout uint node, 
					uint level) {
	float block = voxel_block_extent(level);
	uint P = voxel_block_power(level);
		
	// Calculate brick coordinates and index in block
	vec3 v = vec3(x,y,z);
	vec3 brick = v * block;
	uint child_idx = voxel_brick_index(uvec3(brick), P);
	uint child_offset = node + voxel_node_children_offset(P) + child_idx;

	const uvec2 binary_map_address = voxel_binary_map_address(child_idx);
	const uint binary_map_word_ptr = node + voxel_node_binary_map_offset(P) + binary_map_address.x;

	uint binary_map_word = voxel_buffer[binary_map_word_ptr]; 
	uint bit = 1 << binary_map_address.y;
	
	// Atomically try to set occupancy bit
	if ((binary_map_word & bit) == 0) {
		uint binary_map_word_old_value;
		while ((binary_map_word_old_value = atomicCompSwap(voxel_buffer[binary_map_word_ptr],  
														   binary_map_word,  
														   binary_map_word | bit)) != binary_map_word &&
				(binary_map_word_old_value & bit) == 0) { 
			binary_map_word = binary_map_word_old_value;
		}
		
		// Subdivide the node and create the child if, and only if, we are the first ones here.
		if ((binary_map_word_old_value & bit) == 0) {
			uint child_level = level + 1;
			uint child_size = voxel_node_size(child_level);

			// Allocate memory for child
			uint child_ptr = atomicAdd(voxel_buffer_size, child_size);
			voxel_buffer[child_offset] = child_ptr;

			// Clear child
			if (child_level != voxel_leaf_level) {
				uint child_binary_map_size = voxel_binary_map_size(voxel_block_power(child_level));
				for (int u=0; u<child_binary_map_size; ++u)
					voxel_buffer[child_ptr + u] = 0;
			}
		}
	}

	// Calculate coordinates in brick
	v = fract(brick);
	x = v.x;
	y = v.y;
	z = v.z;
	// And index of pointer to child node
	node = child_offset;
}
