
#include <voxels.glsl>


layout(std430, set=1, binding=0) restrict buffer voxel_buffer_binding {
	uint voxel_buffer[];
};
layout(std430, set=1, binding=1) restrict buffer voxel_counter_binding {
	uint voxel_buffer_size;
};
layout(std430, set=1, binding=2) restrict buffer voxel_list_binding {
	voxel_list_element_t voxel_list_buffer[];
};
layout(std430, set=1, binding=3) restrict buffer voxel_list_counter_binding {
	uint voxel_list_buffer_size;
};


/**
*	@brief	Traverses a node, atomically creating a child at supplied position.
*/
void voxel_voxelize(inout vec3 v,
					inout uint node, 
					uint level) {
	float block = voxel_block_extent(level);
	uint P = voxel_block_power(level);
		
	// Calculate brick coordinates and index in block
	vec3 brick = v * block;
	uint child_idx = voxel_brick_index(ivec3(brick), P);
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
				for (int u=0; u < child_binary_map_size + voxel_ropes_count; ++u)
					voxel_buffer[child_ptr + u] = 0;
			}
		}
	}

	// Calculate coordinates in brick
	v = fract(brick);
	// And index of pointer to child node
	node = child_offset;
}

bool voxel_voxelize_step() {
	return false;
}

/**
*	@brief	Traverses a node, attaching ropes.
*/
void voxel_voxelize_traverse_generating_ropes(vec3 v) {
	uint node = voxel_root_node;
	uint neighbours[voxel_ropes_count];
	for (int i=0; i<voxel_ropes_count; ++i)
		neighbours[i] = voxel_root_node;

	// Handle root node
	int level = 0;
	{
		float block = voxel_block_extent(level);
		uint P = voxel_block_power(level);
		
		// Calculate brick coordinates and index in block
		vec3 brick = v * block;
		uint child_idx = voxel_brick_index(ivec3(brick), P);

		const uvec2 binary_map_address = voxel_binary_map_address(child_idx);
		const uint binary_map_word_ptr = node + voxel_node_binary_map_offset(P) + binary_map_address.x;
	}

	++level;
	float block = voxel_block_extent(level);
	uint P = voxel_block_power(level);

	while (level < voxel_leaf_level) {
		// Calculate brick coordinates and index in block
		vec3 brick = v * block;
		uint child_idx = voxel_brick_index(ivec3(brick), P);

		const uvec2 binary_map_address = voxel_binary_map_address(child_idx);
		const uint binary_map_word_ptr = node + voxel_node_binary_map_offset(P) + binary_map_address.x;
	}
}
