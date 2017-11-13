
#include <common.glsl>
#include <pack.glsl>
#include <material.glsl>

// (2^P)^3 voxels per block
const uint voxel_P = 1;
// Voxel structure end level index
layout(constant_id=1) const uint voxel_leaf_level = 5;

// Voxel world extent
layout(constant_id=2) const float voxel_world = 1000;

// No mip-maps generation under this level
layout(constant_id=3) const uint voxel_min_mipmap_level = 4;

const uint voxel_mask = (1 << voxel_P) - 1;
const uint voxel_mips = voxel_leaf_level - voxel_min_mipmap_level - 1;

const uint voxelizer_work_group_size = 1024;


struct voxel_data_t {
	// Component 0:		3x8-bit RGB albedo, 8-bit counter
	// Component 1:		2x8-bit normal, 8-bit index-of-refraction, 8-bit counter
	// Component 2:		8-bit opacity, 8-bit roughness, 8-bit metallicity, 8-bit counter
	uvec3 packed;
};

struct voxel_unpacked_data_t {
	vec4 albedo;
	vec3 normal;
	float roughness;
	float ior;
	float metallicity;
};

struct voxel_list_element_t {
	voxel_data_t data;
	
	// xy	node index
	// z	node address
	uvec3 voxel_node;
};

struct bricks_list_element_t {
	// Packed node index (21-bits per component)
	uvec2 node;
};


const uint voxel_root_node = 0;


// Size of voxel block
float voxel_tree_block_extent = float(1 << voxel_P);
// Resolution of maximal voxel level
float voxel_grid_resolution = voxel_world / (1 << (voxel_P * voxel_leaf_level));


const uint voxel_buffer_line = 32768;

// Bricks structure:
//  albedo bricks		rgba8_unorm		3x8-bit RGB albedo, 8-bit opacity
//  metadata bricks		rgba8_unorm		2x8-bit normal, 8-bit index-of-refraction, 8-bit metallicity
//  roughness bricks	rg8_unorm		8-bit roughness, 8-bit occupancy (1.0 or 0.0 only)
const uint voxel_brick_line = 4096u;
const uint voxel_brick_max_lines = 2048u;
const uint voxel_bricks_block = 3u;


const float voxel_data_roughness_max_value = .5f;


ivec2 voxels_image_coords(uint ptr) {
	uint x = ptr & (voxel_buffer_line-1);
	uint y = ptr / voxel_buffer_line;
	return ivec2(x,y);
}

ivec3 voxels_brick_image_coords(uint ptr) {
	uint x = ptr & (voxel_brick_line-1);
	uint y = ptr / voxel_brick_line;
	return ivec3(x,y,0) * int(voxel_bricks_block);
}

vec3 voxels_brick_texture_coords(uint ptr, vec3 frac) {
	const vec3 normalizer = vec3(voxel_brick_line, voxel_brick_max_lines, 1) * voxel_bricks_block;
	ivec3 coord = voxels_brick_image_coords(ptr);

	return (vec3(coord) + frac) / normalizer;
}

uvec2 voxels_pack_coordinates(uvec3 v) {
	return v.xy | ((v.zz & uvec2(0x7FF, 0x3FF800)) << uvec2(21, 11));
}
uvec3 voxels_unpack_coordinates(uvec2 u) {
	uvec3 v;
	v.xy = u & 0x1FFFFF;
	v.z = (u.x >> 21) + ((u.y & 0xFFE00000) >> 11);

	return v;
}


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
*	@param	ior			Material index-of-refraction. Clamped to [material_layer_min_ior, material_layer_max_ior] range.
*/
uint encode_voxel_data_normal_ior(vec3 normal, float ior, uint counter) {
	vec2 n = fma(norm3x32_to_snorm2x32(normal), vec2(.5f), vec2(.5f));
    float ior_norm = (ior - material_layer_min_ior) / (material_layer_max_ior - material_layer_min_ior);

	return packUnorm4x8(vec4(n, ior_norm, .0f)) + (min(counter, uint(255)) << 24);
}

/**
*	@brief	Encodes voxel data.
*
*	@param	opacity		Opacity value. Clamped to [0, 1] range.
*	@param	roughness	Material roughness value. Clamped to [0, voxel_data_roughness_max_value] range.
*	@param	metallicity	Material metallicity. Clamped to [0, 1] range.
*/
uint encode_voxel_data_metadata(float opacity, float roughness, float metallicity, uint counter) {
	const vec4 data = vec4(opacity, roughness / voxel_data_roughness_max_value, metallicity, .0f);
	return packUnorm4x8(data) + (min(counter, uint(255)) << 24);
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
	e.packed[1] = encode_voxel_data_normal_ior(normal, ior, 1);
	e.packed[2] = encode_voxel_data_metadata(opacity, roughness, metallicity, 1);

	return e;
}

/**
*	@brief	Decodes voxel information out of the voxel list
*/
vec3 decode_voxel_data_normal(uint data) {
	vec2 unpacked = unpackUnorm4x8(data).xy;
	vec2 n = fma(unpacked, vec2(2.f), vec2(-1.f));

	return snorm2x32_to_norm3x32(n);
}

/**
*	@brief	Decodes voxel information out of the voxel list
*/
float decode_voxel_data_roughness(uint data) {
	return unpackUnorm4x8(data).y * voxel_data_roughness_max_value;
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
	return unpackUnorm4x8(data).x;
}

/**
*	@brief	Decodes voxel information out of the voxel list
*/
float decode_voxel_data_ior(uint data) {
	const float a = unpackUnorm4x8(data).z;
	return mix(material_layer_min_ior, material_layer_max_ior, a);
}

/**
*	@brief	Decodes voxel information out of the voxel list
*/
float decode_voxel_data_metallicity(uint data) {
	return unpackUnorm4x8(data).z;
}

/**
*	@brief	Decodes voxel information out of the voxel list
*/
void decode_voxel_data(voxel_data_t e, out vec3 albedo, out vec3 normal, out float roughness, out float opacity, out float ior, out float metallicity) {
	albedo = decode_voxel_data_albedo(e.packed[0]);
	normal = decode_voxel_data_normal(e.packed[1]);
	opacity = decode_voxel_data_opacity(e.packed[2]);
	roughness = decode_voxel_data_roughness(e.packed[2]);
	ior = decode_voxel_data_ior(e.packed[1]);
	metallicity = decode_voxel_data_metallicity(e.packed[2]);
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
uint voxel_brick_index(uvec3 brick) {
	return brick.z + (((brick.x << voxel_P) + brick.y) << voxel_P);
}

/**
*	@brief	Returns the count of children in a node.
*			Meaningless for leaf nodes.
*/
uint voxel_node_children_count(uint level) {
	return mix(1u << 3 * voxel_P, 0u, level == voxel_leaf_level);
}

/**
*	@brief	Returns the offset of the binary map in a voxel node.
*/
uint voxel_node_binary_map_offset() {
	return 0u;
}

/**
*	@brief	Returns the offset of the brick image address in a voxel node.
*/
uint voxel_node_brick_image_address_offset() {
	return 1u;
}

/**
*	@brief	Returns the offset of the children data in a voxel node.
*/
uint voxel_node_children_offset() {
	return 2u;
}

/**
*	@brief	Returns the size of a voxel node.
*			level must be > 0.
*/
uint voxel_node_size(uint level) {
	return mix(voxel_node_children_offset() + voxel_node_children_count(level), 12u, level == voxel_leaf_level);
}

/** 
*    @brief    Returns the size of the volatile part of the node data that needs to be cleared to 0 upon node initialization. 
*/ 
uint voxel_node_volatile_data_size(uint level) { 
    return voxel_node_size(level);
}
uint voxel_node_volatile_data_offset() { 
    return 0u;
}
