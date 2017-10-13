
// (2^P)^3 voxels per block
layout(constant_id=5) const uint voxel_P = 2;
// (2^Pi)^3 voxels per initial block
layout(constant_id=6) const uint voxel_Pi = 3;
// Voxel structure end level index
layout(constant_id=7) const uint voxel_leaf_level = 5;

// Voxel world extent
layout(constant_id=8) const float voxel_world = 1000;


layout(std430, set=1, binding=0) restrict coherent buffer voxel_buffer_binding {
	uint voxel_buffer[];
};
layout(std430, set=1, binding=1) restrict buffer voxel_counter_binding {
	uint voxel_buffer_size;
};


const uint voxel_lock_pattern = 0xFFFFFFFF;
const uint voxel_root_node = 0;


// Size of voxel block
float voxel_tree_block_extent = 1 << voxel_P;
float voxel_tree_initial_block_extent = 1 << voxel_Pi;
// Resolution of maximal voxel level
float voxel_grid_resolution = voxel_world / ((1 << voxel_Pi) * (1 << voxel_P * (voxel_leaf_level - 1)));

const uint voxel_tree_node_data_size = 0;
const uint voxel_tree_leaf_data_size = 0;

/**
*	@brief	Returns block extent for given voxel level
*/
float voxel_block_extent(uint level) {
	return mix(voxel_tree_block_extent, 
			   voxel_tree_initial_block_extent,
			   level == 0);
}

/**
*	@brief	Returns block power (log2 of extent) for given voxel level
*/
uint voxel_block_power(uint level) {
	return mix(voxel_P, 
			   voxel_Pi, 
			   level == 0);
}

/**
*	@brief	Calculates index of a brick in a block
*/
uint voxel_brick_index(uvec3 brick, uint P) {
	return brick.x + (((brick.z << P) + brick.y) << P);
}

/**
*	@brief	Calculates the address of a brick in the binary map.
*			Returns (word, bit) vector, where word is the 32-bit word index, and bit is the bit index in that word.
*/
uvec2 voxel_binary_map_address(uint brick_index) {
	uint word = brick_index / 32;
	uint bit = brick_index % 32;

	return uvec2(word, bit);
}

/**
*	@brief	Returns the count of children in a node
*/
uint voxel_node_children_count(uint P) {
	return 1 << (3 * P);
}

/**
*	@brief	Returns the binary map size of a node
*/
uint voxel_binary_map_size(uint P) {
	uint map_bits = voxel_node_children_count(P);
	uint map_bytes = map_bits >> 3;
	return map_bytes >> 2;
}

/**
*	@brief	Returns the offset of the semaphore in a voxel node.
*			Meaningless for leaf nodes.
*/
uint voxel_node_semaphore_offset(uint P) {
	return voxel_binary_map_size(P);
}

/**
*	@brief	Returns the offset of the children data in a voxel node.
*			Meaningless for leaf nodes.
*/
uint voxel_node_children_offset(uint P) {
	return voxel_binary_map_size(P) + 1;
}

/**
*	@brief	Returns the offset of the custom data in a voxel node.
*			Meaningless for leaf nodes.
*/
uint voxel_node_data_offset(uint P) {
	return voxel_binary_map_size(P) + 1 + voxel_node_children_count(P);
}

/**
*	@brief	Returns a single child size in a voxel node.
*			level must be > 0.
*/
uint voxel_node_size(uint level) {
	uint P = voxel_block_power(level);
	return mix(voxel_node_data_offset(P) + voxel_tree_node_data_size / 4,
			   voxel_tree_leaf_data_size >> 2,
			   level == voxel_leaf_level);
}
