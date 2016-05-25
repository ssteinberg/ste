
#include "common.glsl"
#include "cook_torrance.glsl"
#include "disney_diffuse.glsl"

struct material_texture_descriptor {
	uint64_t tex_handler;
};
struct material_descriptor {
	material_texture_descriptor basecolor_map;
	material_texture_descriptor cavity_map;
	material_texture_descriptor normal_map;
	material_texture_descriptor mask_map;

	vec3 emission;
	float roughness;
	float anisotropy_ratio;
	float metallic;
	float F0;
	float sheen;
};

const int material_none = 0xFFFFFFFF;

vec3 material_emission(material_descriptor md) {
	return md.emission.rgb;
}

vec3 material_base_color(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy) {
	return md.basecolor_map.tex_handler>0 ? textureGrad(sampler2D(md.basecolor_map.tex_handler), uv, duvdx, duvdy).rgb : vec3(1.f);
}
vec3 material_base_color(material_descriptor md, vec2 uv) {
	return md.basecolor_map.tex_handler>0 ? texture(sampler2D(md.basecolor_map.tex_handler), uv).rgb : vec3(1.f);
}

float material_cavity(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy) {
	return md.cavity_map.tex_handler>0 ? textureGrad(sampler2D(md.cavity_map.tex_handler), uv, duvdx, duvdy).x : 1.f;
}
float material_cavity(material_descriptor md, vec2 uv) {
	return md.cavity_map.tex_handler>0 ? texture(sampler2D(md.cavity_map.tex_handler), uv).x : 1.f;
}

bool material_is_masked(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy) {
	return md.mask_map.tex_handler>0 ? textureGrad(sampler2D(md.mask_map.tex_handler), uv, duvdx, duvdy).x < .5f : false;
}
bool material_is_masked(material_descriptor md, vec2 uv) {
	return md.mask_map.tex_handler>0 ? texture(sampler2D(md.mask_map.tex_handler), uv).x < .5f : false;
}

void normal_map(material_descriptor md, vec2 uv, vec2 duvdx, vec2 duvdy, inout vec3 n, inout vec3 t, inout vec3 b, inout vec3 P) {
	if (md.normal_map.tex_handler > 0) {
		vec3 nm = textureGrad(sampler2D(md.normal_map.tex_handler), uv, duvdx, duvdy).xyz;
		mat3 tbn = mat3(t, b, n);
		n = tbn * normalize(vec3(nm));

		b = cross(t, n);
		t = cross(n, b);
	}
}

vec3 material_evaluate_reflection(material_descriptor md,
								  vec3 n,
								  vec3 t,
								  vec3 b,
								  vec3 v,
								  vec3 l,
								  vec3 base_color,
								  float cavity,
								  vec3 irradiance) {
	vec3 h = normalize(v + l);

	float roughness = md.roughness;
	float metallic = md.metallic;
	float F0 = md.F0;
	float anisotropy_ratio = md.anisotropy_ratio;
	float sheen = md.sheen;

	vec3 specular_tint = vec3(1);
	vec3 sheen_tint = vec3(1);

	vec3 c_spec = F0 * mix(specular_tint, base_color, metallic);

	irradiance *= mix(.2f, 1.f, cavity);

	vec3 S;
	if (anisotropy_ratio != 1.0f) {
		float roughness_x = roughness * anisotropy_ratio;
		float roughness_y = roughness / anisotropy_ratio;

		S = cook_torrance_ansi_brdf(n, t, b,
									v, l, h,
									roughness_x,
									roughness_y,
									c_spec);
	} else {
		S = cook_torrance_iso_brdf(n, v, l, h,
								   roughness,
								   c_spec);
	}

	vec3 D = base_color * disney_diffuse_brdf(n, v, l, h,
											  roughness);

	vec3 c_sheen = fresnel_schlick(dot(l,h)) * sheen_tint * sheen;

	vec3 brdf = S + (1.f - metallic) * (D + c_sheen);
	return brdf * irradiance * dot(n, l);
}
