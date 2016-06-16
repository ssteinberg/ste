
#include "material.glsl"

struct material_layer_unpacked_descriptor {
	vec4 base_color;

	float sheen;
	float sheen_power;
	float anisotropy_ratio;
	float metallic;
	float roughness;
	float thickness;

	float ior;
	float absorption_alpha;
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

material_layer_unpacked_descriptor material_layer_unpack(material_layer_descriptor l, vec2 uv, vec2 duvdx, vec2 duvdy) {
	material_layer_unpacked_descriptor d;

	vec2 sheen_pack = unpackUnorm2x16(l.sheen_pack);
	d.sheen = material_convert_sheen_to_sheen_ratio(sheen_pack.x);
	d.sheen_power = material_convert_sheen_power(sheen_pack.y);

	vec2 ansi_metal_pack = unpackUnorm2x16(l.ansi_metal_pack);
	d.anisotropy_ratio = mix(material_layer_min_ansio_ratio, material_layer_max_ansio_ratio, ansi_metal_pack.x);
	d.metallic = ansi_metal_pack.y;

	vec2 rough_thick_pack = unpackUnorm2x16(l.roughness_thickness_pack);
	d.roughness = rough_thick_pack.x;
	d.thickness = rough_thick_pack.y * material_max_thickness;

	d.ior = l.ior;
	d.absorption_alpha = l.absorption_alpha;

	d.base_color = material_layer_base_color(l, uv, duvdx, duvdy);

	return d;
}
