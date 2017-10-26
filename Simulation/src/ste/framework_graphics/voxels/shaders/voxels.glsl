
#include <common.glsl>
#include <pack.glsl>

// (2^Pi)^3 voxels per initial block
layout(constant_id=2) const uint voxel_Pi = 5;
// (2^P)^3 voxels per block
layout(constant_id=1) const uint voxel_P = 2;
// Voxel structure end level index
layout(constant_id=3) const uint voxel_leaf_level = 4;

// Voxel world extent
layout(constant_id=4) const float voxel_world = 1000;

const uint voxelizer_work_group_size = 1024;


struct voxel_data_t {
	uint albedo_packed;				// 3x8-bit RGB albedo, 8-bit counter
	uint normal_packed;				// 2x12-bit normal, 8-bit counter
	uint opacity_roughness_packed;	// 12-bit opacity, 12-bit roughness, 8-bit counter
};

struct voxel_list_element_t {
	voxel_data_t data;
	
	float node_x, node_y, node_z;
	uint voxel_node;
};


const uint voxel_root_node = 0;


// Size of voxel block
float voxel_tree_block_extent = float(1 << voxel_P);
float voxel_tree_initial_block_extent = float(1 << voxel_Pi);
// Resolution of maximal voxel level
float voxel_grid_resolution = voxel_world / ((1 << voxel_Pi) * (1 << (voxel_P * (voxel_leaf_level - 1))));

const uint voxel_tree_node_data_size = 0;
const uint voxel_tree_leaf_data_size = 12;


const uint voxel_buffer_line = 32768;


/**
*	@brief	Encodes voxel data.
*
*	@param	albedo		RGB albedo value. Must be in [0, 1] range.
*/
uint encode_voxel_data_albedo(vec3 albedo, uint counter) {
	return packUnorm4x8(vec4(albedo, .0f)) + (min(counter, uint(255)) << 24);
}

/**
*	@brief	Encodes voxel data.
*
*	@param	normal		World-space normal
*/
uint encode_voxel_data_normal(vec3 normal, uint counter) {
	uvec2 n = uvec2(round((clamp(norm3x32_to_snorm2x32(normal), vec2(-1), vec2(1)) + 1.f) / 2.f * 4095.f));
	return n.x + (n.y << 12) + (min(counter, uint(255)) << 24);
}

/**
*	@brief	Encodes voxel data.
*
*	@param	opacity		Opacity value. Must be in [0, 1] range.
*	@param	roughness	Material roughness value. Must be in [0, 0.5] range.
*/
uint encode_voxel_data_opacity_roughness(float opacity, float roughness, uint counter) {
	uint roughness_norm = uint(round(clamp(roughness, .0f, .5f) * 2.f * 4095.f)) & 0xFFF;
	uint opacity_norm = uint(round(clamp(opacity, .0f, 1.f) * 4095.f)) & 0xFFF;
	return opacity_norm + (roughness_norm << 12) + (min(counter, uint(255)) << 24);
}

/**
*	@brief	Encodes voxel data.
*
*	@param	normal		World-space normal
*	@param	roughness	Material roughness value. Must be in [0, 0.5] range.
*	@param	albedo		RGB albedo value. Must be in [0, 1] range.
*	@param	opacity		Opacity value. Must be in [0, 1] range.
*/
voxel_data_t encode_voxel_data(vec3 normal, float roughness, vec3 albedo, float opacity) {
	voxel_data_t e;
	e.albedo_packed = encode_voxel_data_albedo(albedo, 1);
	e.normal_packed = encode_voxel_data_normal(normal, 1);
	e.opacity_roughness_packed = encode_voxel_data_opacity_roughness(opacity, roughness, 1);

	return e;
}

/**
*	@brief	Decodes voxel information out of the voxel list
*/
vec3 decode_voxel_data_normal(uint data) {
	uvec2 n = uvec2(data & 0xFFF, (data >> 12) & 0xFFF);
	return snorm2x32_to_norm3x32(vec2(n) * 2.f / 4095.f - 1.f);
}

/**
*	@brief	Decodes voxel information out of the voxel list
*/
float decode_voxel_data_roughness(uint data) {
	return float((data >> 12) & 0xFFF) / 4095.f / 2.f;
}

/**
*	@brief	Decodes voxel information out of the voxel list
*/
vec3 decode_voxel_data_albedo(uint data) {
	return unpackUnorm4x8(data).rgb;
}

/**
*	@brief	Decodes voxel information out of the voxel list
*/
float decode_voxel_data_opacity(uint data) {
	return float(data & 0xFFF) / 4095.f;
}

/**
*	@brief	Decodes voxel information out of the voxel list
*/
void decode_voxel_data(voxel_data_t e, out vec3 normal, out float roughness, out vec3 albedo, out float opacity) {
	normal = decode_voxel_data_normal(e.normal_packed);
	roughness = decode_voxel_data_roughness(e.opacity_roughness_packed);
	opacity = decode_voxel_data_opacity(e.opacity_roughness_packed);
	albedo = decode_voxel_data_albedo(e.albedo_packed);
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
*	@brief	Returns the count of children in a node.
*			Meaningless for leaf nodes.
*/
uint voxel_node_children_count(uint level, uint P) {
	return mix(1 << (3 * P), uint(0), level == voxel_leaf_level);
}

/**
*	@brief	Returns the size of the user data in a voxel node.
*/
uint voxel_node_user_data_size(uint level) {
	return mix(voxel_tree_node_data_size, voxel_tree_leaf_data_size, level == voxel_leaf_level) >> 2;
}

/**
*	@brief	Returns the offset of the custom data in a voxel node.
*/
uint voxel_node_data_offset(uint level, uint P) {
	return uint(0);
}

/**
*	@brief	Returns the offset of the children data in a voxel node.
*/
uint voxel_node_occupancy_offset(uint level, uint P) {
	return voxel_node_data_offset(level, P) + voxel_node_user_data_size(level);
}

/**
*	@brief	Returns the offset of the children data in a voxel node.
*/
uint voxel_node_children_offset(uint level, uint P) {
	return voxel_node_occupancy_offset(level, P) + 1;
}

/**
*	@brief	Returns a single child size in a voxel node.
*			level must be > 0.
*/
uint voxel_node_size(uint level, uint P) {
	return mix(voxel_node_children_offset(level, P) + voxel_node_children_count(level, P), 
			   voxel_node_user_data_size(level),
			   level == voxel_leaf_level);
}

/** 
*    @brief    Returns the size of the volatile part of the node data that needs to be cleared to 0 upon node initialization. 
*/ 
uint voxel_node_volatile_data_size(uint level, uint P) { 
    return voxel_node_size(level, P);
}
uint voxel_node_volatile_data_offset(uint level, uint P) { 
    return uint(0);
}
