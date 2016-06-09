
#include "common.glsl"
#include "cook_torrance.glsl"
#include "disney_diffuse.glsl"

struct material_texture_descriptor {
	uint64_t tex_handler;
};

struct material_descriptor {
	material_texture_descriptor cavity_map;
	material_texture_descriptor normal_map;
	material_texture_descriptor mask_map;
	material_texture_descriptor _unused;

	vec3 emission;

	uint32_t head_layer;
};

struct material_layer_descriptor {
	material_texture_descriptor basecolor_map;
	
	float thickness;

	float roughness;
	float anisotropy_ratio;
	float metallic;
	float ior;

	float sheen;
	float sheen_power;

	uint32_t next_layer;

	float _unused[2];
};

const int material_none = 0xFFFFFFFF;
const float material_cavity_min = .2f;
const float material_cavity_max = 1.f;
const float material_alpha_discard_threshold = .5f;

float material_convert_ior_to_F0(float ior1, float ior2) {
	float t = (ior1 - ior2) / (ior1 + ior2);
	return t * t;
} 

vec3 material_emission(material_descriptor md) {
	return md.emission.rgb;
}

vec3 material_layer_base_color(material_layer_descriptor layer, vec2 uv, vec2 duvdx, vec2 duvdy) {
	if (layer.basecolor_map.tex_handler > 0)
		return textureGrad(sampler2D(layer.basecolor_map.tex_handler), uv, duvdx, duvdy).rgb;
	return vec3(1.f);
}
vec3 material_layer_base_color(material_layer_descriptor layer, vec2 uv) {
	if (layer.basecolor_map.tex_handler > 0)
		return texture(sampler2D(layer.basecolor_map.tex_handler), uv).rgb;
	return vec3(1.f);
}

float material_cavity(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy) {
	if (md.cavity_map.tex_handler > 0)
		return mix(material_cavity_min, material_cavity_max, textureGrad(sampler2D(md.cavity_map.tex_handler), uv, duvdx, duvdx).x);
	return 1.f;
}
float material_cavity(material_descriptor md, vec2 uv) {
	if (md.cavity_map.tex_handler > 0)
		return mix(material_cavity_min, material_cavity_max, texture(sampler2D(md.cavity_map.tex_handler), uv).x);
	return 1.f;
}

bool material_is_masked(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy) {
	if (md.mask_map.tex_handler > 0)
		return textureGrad(sampler2D(md.mask_map.tex_handler), uv, duvdx, duvdy).x < material_alpha_discard_threshold;
	return false;
}
bool material_is_masked(material_descriptor md, vec2 uv) {
	if (md.mask_map.tex_handler > 0)
		return texture(sampler2D(md.mask_map.tex_handler), uv).x < material_alpha_discard_threshold;
	return false;
}
