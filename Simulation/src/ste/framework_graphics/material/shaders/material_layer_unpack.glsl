
#include "material.glsl"

struct material_layer_unpacked_descriptor {
	vec4 color;

	float anisotropy_ratio;
	float metallic;
	float roughness;
	float thickness;

	float ior;
	vec3 attenuation_coefficient;
	float phase_g;
};

float material_convert_ior_to_F0(float ior1, float ior2) {
	float t = (ior1 - ior2) / (ior1 + ior2);
	return t * t;
} 

float material_convert_sheen_to_sheen_ratio(float s) {
	return s * 4;
}

float material_convert_sheen_power(float s) {
	return mix(5.f, 2.f, s);
}

material_layer_unpacked_descriptor material_layer_unpack(material_layer_descriptor l) {
	material_layer_unpacked_descriptor d;

	vec2 ansi_metal_pack = unpackUnorm2x16(l.ansi_metal_pack);
	d.anisotropy_ratio = mix(material_layer_min_ansio_ratio, material_layer_max_ansio_ratio, ansi_metal_pack.x);
	d.metallic = ansi_metal_pack.y;

	vec2 rough_thick_pack = unpackUnorm2x16(l.roughness_thickness_pack);
	d.roughness = rough_thick_pack.x;
	d.thickness = rough_thick_pack.y * material_max_thickness;

	d.ior = l.ior;

	vec4 unpacked_att_coefficients_and_phase = unpackUnorm4x8(l.packed_attenuation_coefficient_rgb_phase);
	d.attenuation_coefficient = unpacked_att_coefficients_and_phase.xyz * l.attenuation_coefficient_scale;
	d.phase_g = mix(-1.f, +1.f, unpacked_att_coefficients_and_phase.w);

	d.color = unpackUnorm4x8(l.packed_color);

	return d;
}
