
#include <material.glsl>

struct material_layer_unpacked_descriptor {
	vec4 albedo;

	//float anisotropy_ratio;
	float metallic;
	float roughness;
	float thickness;
	float ior;
	
	vec3 attenuation_coefficient;
	float phase_g;

	uint next_layer_id;
};

bool material_is_base_layer(material_layer_unpacked_descriptor desc) {
	return desc.next_layer_id == material_none;
}

material_layer_unpacked_descriptor material_layer_unpack(material_layer_descriptor l, vec2 uv, vec2 duvdx, vec2 duvdy) {
	material_layer_unpacked_descriptor d;

	d.albedo = unpackUnorm4x8(l.packed_albedo);
	d.next_layer_id = l.next_layer_id;
	d.attenuation_coefficient = l.attenuation_coefficient.rgb;
	
	d.roughness = textureGrad(sampler2D(material_textures[l.roughness_sampler_idx], material_sampler), 
							  uv, 
							  duvdx, 
							  duvdy).x;
	d.metallic =  textureGrad(sampler2D(material_textures[l.metallicity_sampler_idx], material_sampler), 
							  uv, 
							  duvdx, 
							  duvdy).x;
	d.thickness = textureGrad(sampler2D(material_textures[l.thickness_sampler_idx], material_sampler), 
							  uv, 
							  duvdx, 
							  duvdy).x;

	vec2 ior_phase_pack = unpackUnorm2x16(l.ior_phase_pack);
	d.ior = mix(material_layer_min_ior, material_layer_max_ior, ior_phase_pack.x);
	d.phase_g = ior_phase_pack.y * 2.f - 1.f;

	return d;
}
