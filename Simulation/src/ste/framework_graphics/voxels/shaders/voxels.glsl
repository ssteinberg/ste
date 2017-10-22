
#include <common.glsl>
#include <pack.glsl>

// (2^Pi)^3 voxels per initial block
layout(constant_id=2) const uint voxel_Pi = 4;
// (2^P)^3 voxels per block
layout(constant_id=1) const uint voxel_P = 2;
// Voxel structure end level index
layout(constant_id=3) const uint voxel_leaf_level = 4;

// Voxel world extent
layout(constant_id=4) const float voxel_world = 1000;

const uint voxelizer_work_group_size = 1024;


struct voxel_list_element_t {
	float x,y,z;
	uint normal_and_material;
	uint voxel_node;
};


const uint voxel_root_node = 0;


// Size of voxel block
float voxel_tree_block_extent = float(1 << voxel_P);
float voxel_tree_initial_block_extent = float(1 << voxel_Pi);
// Resolution of maximal voxel level
float voxel_grid_resolution = voxel_world / ((1 << voxel_Pi) * (1 << (voxel_P * (voxel_leaf_level - 1))));

const uint voxel_tree_node_data_size = 0;
const uint voxel_tree_leaf_data_size = 8;

/**
*	@brief	Encodes a voxel into voxel list
*/
void encode_voxel_list_element(out voxel_list_element_t e, vec3 P, vec3 N, uint matrial_id) {
	vec3 voxel_position = P;
	uvec2 n = uvec2(round((clamp(norm3x32_to_snorm2x32(N), vec2(-1), vec2(1)) + 1.f) * 255.f / 2.f));

	e.normal_and_material = n.x + (n.y << 8) + ((matrial_id & 0xFFFF) << 16);
	e.x = voxel_position.x;
	e.y = voxel_position.y;
	e.z = voxel_position.z;
}

/**
*	@brief	Decodes voxel information out of the voxel list
*/
void decode_voxel_list_element(voxel_list_element_t e, out vec3 N, out uint matrial_id) {
	matrial_id = e.normal_and_material >> 16;

	uvec2 n = uvec2(e.normal_and_material & 0xFF, (e.normal_and_material >> 8) & 0xFF);
	N = snorm2x32_to_norm3x32(vec2(n) * 2.f / 255.f - 1.f);
}

/**
*	@brief	Computes resolution of a given voxel level
*/
uint voxel_resolution(uint level) {
	uint p = voxel_Pi + voxel_P * level;
	return 1 << p;
}

/**
*	@brief	Computes resolution difference between given voxel levels.
*			Expects: level1 >= level0
*/
uint voxel_resolution_difference(uint level0, uint level1) {
	uint p = voxel_P * (level1 - level0);
	return 1 << p;
}

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
uint voxel_brick_index(ivec3 brick, uint P) {
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
*	@brief	Returns the offset of the binary map in a voxel node.
*			Meaningless for leaf nodes.
*/
uint voxel_node_binary_map_offset(uint P) {
	return 0;
}

/**
*	@brief	Returns the offset of the children data in a voxel node.
*			Meaningless for leaf nodes.
*/
uint voxel_node_children_offset(uint P) {
	return voxel_binary_map_size(P);
}

/**
*	@brief	Returns the offset of the custom data in a voxel node.
*			Meaningless for leaf nodes.
*/
uint voxel_node_data_offset(uint P) {
	return voxel_binary_map_size(P) + voxel_node_children_count(P);
}

/**
*	@brief	Returns a single child size in a voxel node.
*			level must be > 0.
*/
uint voxel_node_size(uint level) {
	uint P = voxel_block_power(level);
	return mix(voxel_node_data_offset(P) + (voxel_tree_node_data_size >> 2),
			   voxel_tree_leaf_data_size >> 2,
			   level == voxel_leaf_level);
}
