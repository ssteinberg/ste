
#include <common.glsl>
#include <pack.glsl>
#include <material.glsl>

// (2^P)^3 voxels per block
const uint voxel_P = 1;
// Voxel structure end level index
layout(constant_id=1) const uint voxel_leaf_level = 5;

// Voxel world extent
layout(constant_id=2) const float voxel_world = 1000;

const uint voxelizer_work_group_size = 1024;


struct voxel_data_t {
	// Component 0:		3x8-bit RGB albedo, 8-bit counter
	// Component 1:		2x12-bit normal, 8-bit counter
	// Component 2:		12-bit opacity, 12-bit roughness, 8-bit counter
	// Component 3:		12-bit index-of-refraction, 12-bit metallicity, 8-bit counter
	uvec4 packed;
};

struct voxel_list_element_t {
	voxel_data_t data;

	vec3 voxel_node_position;
	uint voxel_node;
};


const uint voxel_root_node = 0;


// Size of voxel block
float voxel_tree_block_extent = float(1 << voxel_P);
// Resolution of maximal voxel level
float voxel_grid_resolution = voxel_world / (1 << (voxel_P * voxel_leaf_level));

const uint voxel_tree_node_data_size = 0;
const uint voxel_tree_leaf_data_size = 16;


const uint voxel_buffer_line = 32768;


const float voxel_data_roughness_max_value = .5f;


/**
*	@brief	Encodes voxel data.
*
*	@param	albedo		RGB albedo value. Clamped to [0, 1] range.
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
*	@param	opacity		Opacity value. Clamped to [0, 1] range.
*	@param	roughness	Material roughness value. Clamped to [0, voxel_data_roughness_max_value] range.
*/
uint encode_voxel_data_opacity_roughness(float opacity, float roughness, uint counter) {
    uint opacity_norm = uint(round(clamp(opacity, .0f, 1.f) * 4095.f)) & 0xFFF;
    uint roughness_norm = uint(round(clamp(roughness, .0f, voxel_data_roughness_max_value) / voxel_data_roughness_max_value * 4095.f)) & 0xFFF;

    return opacity_norm + (roughness_norm << 12) + (min(counter, uint(255)) << 24); 
}

/**
*	@brief	Encodes voxel data.
*
*	@param	ior			Material index-of-refraction. Clamped to [material_layer_min_ior, material_layer_max_ior] range.
*	@param	metallicity	Material metallicity. Clamped to [0, 1] range.
*/
uint encode_voxel_data_ior_metallicity(float ior, float metallicity, uint counter) {
    uint ior_norm = uint(round(clamp((ior - material_layer_min_ior) / (material_layer_max_ior - material_layer_min_ior), .0f, 1.f) * 4095.f)) & 0xFFF;
    uint metallicity_norm = uint(round(clamp(metallicity, .0f, 1.f) * 4095.f)) & 0xFFF;

    return ior_norm + (metallicity_norm << 12) + (min(counter, uint(255)) << 24);
}

/**
*	@brief	Encodes voxel data.
*
*	@param	normal		World-space normal
*	@param	opacity		Opacity value. Clamped to [0, 1] range.
*	@param	roughness	Material roughness value. Clamped to [0, voxel_data_roughness_max_value] range.
*	@param	ior			Material index-of-refraction. Clamped to [material_layer_min_ior, material_layer_max_ior] range.
*	@param	metallicity	Material metallicity. Clamped to [0, 1] range.
*/
voxel_data_t encode_voxel_data(vec3 normal, float roughness, vec3 albedo, float opacity, float ior, float metallicity) {
	voxel_data_t e;
	e.packed[0] = encode_voxel_data_albedo(albedo, 1);
	e.packed[1] = encode_voxel_data_normal(normal, 1);
	e.packed[2] = encode_voxel_data_opacity_roughness(opacity, roughness, 1);
	e.packed[3] = encode_voxel_data_ior_metallicity(ior, metallicity, 1);

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
	return float((data >> 12) & 0xFFF) / 4095.f * voxel_data_roughness_max_value;
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
float decode_voxel_data_ior(uint data) {
	const float a = float(data & 0xFFF) / 4095.f;
	return mix(material_layer_min_ior, material_layer_max_ior, a);
}

/**
*	@brief	Decodes voxel information out of the voxel list
*/
float decode_voxel_data_metallicity(uint data) {
	return float((data >> 12) & 0xFFF) / 4095.f;
}

/**
*	@brief	Decodes voxel information out of the voxel list
*/
void decode_voxel_data(voxel_data_t e, out vec3 albedo, out vec3 normal, out float roughness, out float opacity, out float ior, out float metallicity) {
	albedo = decode_voxel_data_albedo(e.packed[0]);
	normal = decode_voxel_data_normal(e.packed[1]);
	opacity = decode_voxel_data_opacity(e.packed[2]);
	roughness = decode_voxel_data_roughness(e.packed[2]);
	ior = decode_voxel_data_ior(e.packed[3]);
	metallicity = decode_voxel_data_metallicity(e.packed[3]);
}

/**
*	@brief	Computes resolution of a given voxel level
*/
uint voxel_resolution(uint level) {
	uint p = voxel_P * (level + 1);
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
	return voxel_tree_block_extent;
}

/**
*	@brief	Returns block power (log2 of extent) for given voxel level
*/
uint voxel_block_power(uint level) {
	return voxel_P;
}

/**
*	@brief	Calculates index of a brick in a block
*/
uint voxel_brick_index(ivec3 brick) {
	return brick.z + (((brick.x << voxel_P) + brick.y) << voxel_P);
}

/**
*	@brief	Returns the count of children in a node.
*			Meaningless for leaf nodes.
*/
uint voxel_node_children_count(uint level) {
	return mix(1 << 3 * voxel_P, uint(0), level == voxel_leaf_level);
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
uint voxel_node_data_offset(uint level) {
	return uint(0);
}

/**
*	@brief	Returns the offset of the children data in a voxel node.
*/
uint voxel_node_children_offset(uint level) {
	return 0;
}

/**
*	@brief	Returns the size of a voxel node.
*			level must be > 0.
*/
uint voxel_node_size(uint level) {
	return mix(voxel_node_children_offset(level) + voxel_node_children_count(level), 
			   voxel_node_user_data_size(level),
			   level == voxel_leaf_level);
}

/** 
*    @brief    Returns the size of the volatile part of the node data that needs to be cleared to 0 upon node initialization. 
*/ 
uint voxel_node_volatile_data_size(uint level) { 
    return voxel_node_size(level);
}
uint voxel_node_volatile_data_offset(uint level) { 
    return uint(0);
}
