
#include <common.glsl>

struct material_texture_descriptor {
	uint sampler_idx;
};

const uint material_has_texture_bit = 0x1 << 0;
const uint material_has_cavity_map_bit = 0x1 << 1;
const uint material_has_normal_map_bit = 0x1 << 2;
const uint material_has_mask_map_bit = 0x1 << 3;

const uint material_has_subsurface_scattering_bit = 0x1 << 31;

struct material_descriptor {
	material_texture_descriptor cavity_map;
	material_texture_descriptor normal_map;
	material_texture_descriptor mask_map;
	material_texture_descriptor texture;
	
	float emission;
	uint packed_emission_color;
	
	uint head_layer;

	uint material_flags;
};

struct material_layer_descriptor {
	uint roughness_sampler_idx;
	uint metallicity_sampler_idx;
	uint thickness_sampler_idx;
	
	uint next_layer_id;
	
	vec4 attenuation_coefficient;
	
	uint packed_albedo;
	uint ior_phase_pack;
	
	uint _unused1;
	uint _unused2;
};

layout(std430, set=2, binding=4) restrict readonly buffer material_descriptors_binding {
	material_descriptor mat_descriptor[];
};
layout(std430, set=2, binding=5) restrict readonly buffer material_layer_descriptors_binding {
	material_layer_descriptor mat_layer_descriptor[];
};
layout(constant_id=0) const int material_textures_count = 2;
layout(set=2, binding=13) uniform texture2D material_textures[material_textures_count];
layout(set=2, binding=14) uniform sampler material_sampler;

const int material_none = 0xFFFFFFFF;

const float material_cavity_min = .2f;
const float material_cavity_max = 1.f;

const float material_alpha_discard_threshold = .5f;

const float material_max_thickness = .1f;

const float material_layer_ansio_ratio_scale = .9f;
const float material_layer_max_ansio_ratio = 1.f / sqrt(1.f - 1.f * material_layer_ansio_ratio_scale);
const float material_layer_min_ansio_ratio = sqrt(1.f - 1.f * material_layer_ansio_ratio_scale);

const float material_layer_min_ior = 1.f;
const float material_layer_max_ior = 5.f;

/**
*	@brief	Returns material's emission value
*/
vec3 material_emission(material_descriptor md) {
	vec3 emission_color = unpackUnorm4x8(md.packed_emission_color).rgb;
	return emission_color * md.emission;
}

/**
*	@brief	Returns material's albedo
*/
vec4 material_base_texture(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy) {
	if ((md.material_flags & material_has_texture_bit) != 0)
		return textureGrad(sampler2D(material_textures[md.texture.sampler_idx], material_sampler), 
						   uv, 
						   duvdx, 
						   duvdy);
	return vec4(1.f);
}
/**
*	@brief	Returns material's albedo
*/
vec4 material_base_texture(material_descriptor md, vec2 uv) {
	if ((md.material_flags & material_has_texture_bit) != 0)
		return texture(sampler2D(material_textures[md.texture.sampler_idx], material_sampler), uv);
	return vec4(1.f);
}

/**
*	@brief	Returns material's cavity value
*/
float material_cavity(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy) {
	if ((md.material_flags & material_has_cavity_map_bit) != 0) {
		float t = textureGrad(sampler2D(material_textures[md.cavity_map.sampler_idx], material_sampler), uv, duvdx, duvdx).x;
		return mix(material_cavity_min, material_cavity_max, t);
	}
	return 1.f;
}
/**
*	@brief	Returns material's cavity value
*/
float material_cavity(material_descriptor md, vec2 uv) {
	if ((md.material_flags & material_has_cavity_map_bit) != 0) {
		float t = texture(sampler2D(material_textures[md.cavity_map.sampler_idx], material_sampler), uv).x;
		return mix(material_cavity_min, material_cavity_max, t);
	}
	return 1.f;
}

/**
*	@brief	Returns material's opacity
*/
float material_opacity(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy) {
	if ((md.material_flags & material_has_mask_map_bit) != 0)
		return textureGrad(sampler2D(material_textures[md.mask_map.sampler_idx], material_sampler), 
						   uv, 
						   duvdx, 
						   duvdy).x;
	return 1.f;
}
/**
*	@brief	Returns material's opacity
*/
float material_opacity(material_descriptor md, vec2 uv) {
	if ((md.material_flags & material_has_mask_map_bit) != 0)
		return texture(sampler2D(material_textures[md.mask_map.sampler_idx], material_sampler), uv).x;
	return 1.f;
}

/**
*	@brief	Check if the material is masked, i.e. opacity is below a threshold.
*/
bool material_is_masked(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy) {
	return material_opacity(md, 
							uv, 
							duvdx, 
							duvdy).x < material_alpha_discard_threshold;
}
/**
*	@brief	Check if the material is masked, i.e. opacity is below a threshold.
*/
bool material_is_masked(material_descriptor md, vec2 uv) {
	return material_opacity(md, 
							uv).x < material_alpha_discard_threshold;
}

/**
*	@brief	Check if the material has subsurface-scattering enabled
*/
bool material_has_subsurface_scattering(material_descriptor md) {
	return (md.material_flags & material_has_subsurface_scattering_bit) != 0;
}

/**
*	@brief	Check if the material is "simple", i.e. has only a single layer and no subsurface-scattering.
*/
bool material_is_simple(material_descriptor md, material_layer_descriptor head_layer) {
	return !material_has_subsurface_scattering(md) && 
			head_layer.next_layer_id == material_none;
}

/**
*	@brief	Given an orthonormal frame, transforms it based on the material's normal map.
*/
void normal_map(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy, inout vec3 n, inout vec3 t, inout vec3 b) {
	if ((md.material_flags & material_has_normal_map_bit) != 0) {
		mat3 transform = mat3(t, b, n);

		vec3 nm;
		nm.xy = textureGrad(sampler2D(material_textures[md.normal_map.sampler_idx], material_sampler), 
							uv, 
							duvdx, 
							duvdy).xy * 2.f - 1.f;
		nm.y *= -1.f;
		nm.z = sqrt(1.f - clamp(dot(nm.xy,nm.xy), .0f, 1.f));
		
		n = transform * normalize(nm);
		b = normalize(cross(t, n));
		t = cross(n, b);
	}
}
