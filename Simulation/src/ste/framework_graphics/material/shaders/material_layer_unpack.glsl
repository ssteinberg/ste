
#include "material.glsl"

struct material_layer_unpacked_descriptor {
	vec3 base_color;

	float sheen;
	float sheen_power;
	float anisotropy_ratio;
	float metallic;
	float roughness;
	float thickness;

	float ior;
	float absorption_alpha;

	uint32_t next_layer_id;
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

	vec2 pack;
	
	pack = unpackUnorm2x16(l.sheen_pack);
	d.sheen = material_convert_sheen_to_sheen_ratio(pack.x);
	d.sheen_power = material_convert_sheen_power(pack.y);

	pack = unpackUnorm2x16(l.ansi_metal_pack);
	d.anisotropy_ratio = pack.x * 2.f - 1.f;
	d.metallic = pack.y;

	pack = unpackUnorm2x16(l.roughness_thickness_pack);
	d.roughness = pack.x;
	d.thickness = pack.y * material_max_thickness;

	d.ior = l.ior;
	d.absorption_alpha = d.absorption_alpha;
	d.next_layer_id = d.next_layer_id;

	d.base_color = material_layer_base_color(l, uv, duvdx, duvdy);

	return d;
}
