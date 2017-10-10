
// Size of voxel block
vec3 voxel_tree_block_extent = vec3(1 << voxel_P);
vec3 voxel_tree_initial_block_extent = vec3(1 << voxel_Pi);
// Resolution of maximal voxel level
float voxel_grid_resolution = voxel_world / ((1 << voxel_Pi) * (1 << voxel_P * (voxel_leaf_level - 1)));

const uint voxel_tree_node_data_size = 0;
const uint voxel_tree_leaf_data_size = 0;

/**
*	@brief	Returns block extent for given voxel level
*/
vec3 voxel_block_extent(uint level) {
	return mix(voxel_tree_block_extent, 
			   voxel_tree_initial_block_extent,
			   bvec3(level == 0));
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
*	@brief	Returns the offset of the parent data in a voxel node.
*			Meaningless for leaf nodes.
*/
uint voxel_node_parent_offset(uint P) {
	return (1 << (3 * P - 5));
}

/**
*	@brief	Returns the offset of the children data in a voxel node.
*			Meaningless for leaf nodes.
*/
uint voxel_node_children_offset(uint P) {
	return (1 << (3 * P - 5)) + voxel_tree_node_data_size / 4 + 1;
}

/**
*	@brief	Returns the offset of the custom data in a voxel node.
*			Meaningless for leaf nodes.
*/
uint voxel_node_data_offset(uint P) {
	return 1 << (3 * P - 5) + voxel_tree_node_data_size / 4;
}

/**
*	@brief	Returns the node size.
			level must be > 0.
*/
uint voxel_node_size(uint level) {
	return mix((1 << (3 * voxel_P - 5)) * 33 + voxel_tree_node_data_size / 4 + 1,
			   voxel_tree_leaf_data_size >> 2,
			   level == voxel_leaf_level);
}
