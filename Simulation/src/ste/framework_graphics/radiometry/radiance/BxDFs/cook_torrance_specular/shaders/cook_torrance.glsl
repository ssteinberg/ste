
#include "common.glsl"
#include "microfacet.glsl"

vec3 cook_torrance_iso_brdf(vec3 n,
							vec3 v,
							vec3 l,
							vec3 h,
							float roughness,
							float F0,
							vec3 c_spec,
							out float D,
							out float G,
							out float F) {
	// Singularity at "grazing-angles", i.e. dot(n,v) == 0
	float clamped_dotNL = max(epsilon, dot(n,l));
	float clamped_dotNV = max(epsilon, dot(n,v));

	D = ndf_ggx_isotropic(roughness, dot(n,h));
	G = gaf_schlick_ggx(roughness, clamped_dotNL, clamped_dotNV);
	F = fresnel_schlick(F0, dot(l,h));

	return c_spec * max(.0f, D * G * F / 4.f);
}

vec3 cook_torrance_ansi_brdf(vec3 n,
							 vec3 t,
							 vec3 b,
							 vec3 v,
							 vec3 l,
							 vec3 h,
							 float roughness_x,
							 float roughness_y,
							 float F0,
							 vec3 c_spec,
							 out float D,
							 out float G,
							 out float F) {
	// Like the isotropic case
	float clamped_dotNL = max(epsilon, dot(n,l));
	float clamped_dotNV = max(epsilon, dot(n,v));

	float ansi_ratio = clamp(dot(t,h) / (dot(t,h) + dot(b,h)), .0f, 1.f);
	float roughness = mix(roughness_y, roughness_x, ansi_ratio);

	D = ndf_ggx_ansiotropic(t, b, h, roughness_x, roughness_y, dot(n,h));
	G = gaf_schlick_ggx(roughness, clamped_dotNL, clamped_dotNV);
	F = fresnel_schlick(F0, dot(l,h));

	return c_spec * max(.0f, D * G * F / 4.f);
}
