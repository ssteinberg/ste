
#include "material.glsl"

struct material_layer_unpacked_descriptor {
	vec4 color;

	float anisotropy_ratio;
	float metallic;
	float roughness;
	float thickness;

	float ior;
	float phase_g;
	vec3 attenuation_coefficient;

	uint32_t next_layer_id;
};

float material_convert_ior_to_F0(float sin_critical) {
	float t = (1.f - sin_critical) / (1.f + sin_critical);
	return t * t;
}

bool material_is_base_layer(material_layer_unpacked_descriptor desc) {
	return desc.next_layer_id == material_none;
}

material_layer_unpacked_descriptor material_layer_unpack(material_layer_descriptor l) {
	material_layer_unpacked_descriptor d;

	d.color = unpackUnorm4x8(l.packed_color);
	d.next_layer_id = l.next_layer_id;
	d.attenuation_coefficient = l.attenuation_coefficient;

	vec2 ansi_metal_pack = unpackUnorm2x16(l.ansi_metal_pack);
	d.anisotropy_ratio = mix(material_layer_min_ansio_ratio, material_layer_max_ansio_ratio, ansi_metal_pack.x);
	d.metallic = ansi_metal_pack.y;

	vec2 rough_thick_pack = unpackUnorm2x16(l.roughness_thickness_pack);
	d.roughness = rough_thick_pack.x;
	d.thickness = rough_thick_pack.y * material_max_thickness;

	vec2 ior_phase_pack = unpackUnorm2x16(l.ior_phase_pack);
	d.ior = mix(material_layer_min_ior, material_layer_max_ior, ior_phase_pack.x);
	d.phase_g = ior_phase_pack.y * 2.f - 1.f;

	return d;
}
