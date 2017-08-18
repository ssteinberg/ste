
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
	
	vec3 attenuation_coefficient;
	uint _unused0;
	
	uint packed_albedo;
	uint ior_phase_pack;
	
	uint _unused1;
	uint _unused2;
};

layout(std430, set=0, binding=1) restrict readonly buffer material_descriptors_binding {
	material_descriptor mat_descriptor[];
};
layout(std430, set=0, binding=2) restrict buffer material_layer_descriptors_binding {
	material_layer_descriptor mat_layer_descriptor[];
};
layout(set=0, binding=3) uniform sampler2D material_samplers[10000];

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
	if ((md.material_flags & material_has_texture_bit) != 0)
		return textureGrad(material_samplers[md.texture.sampler_idx], uv, duvdx, duvdy);
	return vec4(1.f);
}
vec4 material_base_texture(material_descriptor md, vec2 uv) {
	if ((md.material_flags & material_has_texture_bit) != 0)
		return texture(material_samplers[md.texture.sampler_idx], uv);
	return vec4(1.f);
}

float material_cavity(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy) {
	if ((md.material_flags & material_has_cavity_map_bit) != 0)
		return mix(material_cavity_min, material_cavity_max, textureGrad(material_samplers[md.cavity_map.sampler_idx], uv, duvdx, duvdx).x);
	return 1.f;
}
float material_cavity(material_descriptor md, vec2 uv) {
	if ((md.material_flags & material_has_cavity_map_bit) != 0)
		return mix(material_cavity_min, material_cavity_max, texture(material_samplers[md.cavity_map.sampler_idx], uv).x);
	return 1.f;
}

bool material_is_masked(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy) {
	if ((md.material_flags & material_has_mask_map_bit) != 0)
		return textureGrad(material_samplers[md.mask_map.sampler_idx], uv, duvdx, duvdy).x < material_alpha_discard_threshold;
	return false;
}
bool material_is_masked(material_descriptor md, vec2 uv) {
	if ((md.material_flags & material_has_mask_map_bit) != 0)
		return texture(material_samplers[md.mask_map.sampler_idx], uv).x < material_alpha_discard_threshold;
	return false;
}

bool material_has_subsurface_scattering(material_descriptor md) {
	return (md.material_flags & material_has_subsurface_scattering_bit) != 0;
}

bool material_is_simple(material_descriptor md, material_layer_descriptor head_layer) {
	return !material_has_subsurface_scattering(md) && 
			head_layer.next_layer_id == material_none;
}

void normal_map(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy, inout vec3 n, inout vec3 t, inout vec3 b) {
	if ((md.material_flags & material_has_normal_map_bit) != 0) {
		mat3 transform = mat3(t, b, n);

		vec3 nm;
		nm.xy = textureGrad(material_samplers[md.normal_map.sampler_idx], uv, duvdx, duvdy).xy * 2.f - 1.f;
		nm.z = sqrt(1.f - clamp(dot(nm.xy,nm.xy), .0f, 1.f));
		
		n = transform * normalize(nm);
		b = normalize(cross(t, n));
		t = cross(n, b);
	}
}
