
#include "common.glsl"

struct material_texture_descriptor {
	layout(bindless_sampler) sampler2D tex_handler;
};

const uint material_has_texture = 0x1 << 0;
const uint material_has_cavity_map = 0x1 << 1;
const uint material_has_normal_map = 0x1 << 2;
const uint material_has_mask_map = 0x1 << 3;

struct material_descriptor {
	material_texture_descriptor cavity_map;
	material_texture_descriptor normal_map;
	material_texture_descriptor mask_map;
	material_texture_descriptor texture;
	
	float emission;
	uint packed_emission_color;
	
	uint head_layer;

	uint used_textures_mask;
};

struct material_layer_descriptor {
	uint packed_albedo;

	uint ansi_metal_pack;
	uint roughness_thickness_pack;
	
	uint next_layer_id;
	
	vec3 attenuation_coefficient;
	uint ior_phase_pack;
};

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

vec3 material_emission(material_descriptor md) {
	vec3 emission_color = unpackUnorm4x8(md.packed_emission_color).rgb;
	return emission_color * md.emission;
}

vec4 material_base_texture(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy) {
	if ((md.used_textures_mask & material_has_texture) != 0)
		return textureGrad(md.texture.tex_handler, uv, duvdx, duvdy);
	return vec4(1.f);
}
vec4 material_base_texture(material_descriptor md, vec2 uv) {
	if ((md.used_textures_mask & material_has_texture) != 0)
		return texture(md.texture.tex_handler, uv);
	return vec4(1.f);
}

float material_cavity(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy) {
	if ((md.used_textures_mask & material_has_cavity_map) != 0)
		return mix(material_cavity_min, material_cavity_max, textureGrad(md.cavity_map.tex_handler, uv, duvdx, duvdx).x);
	return 1.f;
}
float material_cavity(material_descriptor md, vec2 uv) {
	if ((md.used_textures_mask & material_has_cavity_map) != 0)
		return mix(material_cavity_min, material_cavity_max, texture(md.cavity_map.tex_handler, uv).x);
	return 1.f;
}

bool material_is_masked(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy) {
	if ((md.used_textures_mask & material_has_mask_map) != 0)
		return textureGrad(md.mask_map.tex_handler, uv, duvdx, duvdy).x < material_alpha_discard_threshold;
	return false;
}
bool material_is_masked(material_descriptor md, vec2 uv) {
	if ((md.used_textures_mask & material_has_mask_map) != 0)
		return texture(md.mask_map.tex_handler, uv).x < material_alpha_discard_threshold;
	return false;
}

void normal_map(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy, inout vec3 n, inout vec3 t, inout vec3 b) {
	if ((md.used_textures_mask & material_has_normal_map) != 0) {
		mat3 tbn = mat3(t, b, n);

		vec3 nm = textureGrad(md.normal_map.tex_handler, uv, duvdx, duvdy).xyz;
		n = tbn * normalize(nm);

		b = cross(t, n);
		t = cross(n, b);
	}
}
