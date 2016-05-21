
#include "common.glsl"
#include "microfacet.glsl"

vec3 cook_torrance_iso_brdf(vec3 n,
							vec3 v,
							vec3 l,
							vec3 h,
							float roughness,
							vec3 c_spec) {
	// Singularity at "grazing-angles", i.e. dot(n,v) == 0
	float clamped_dotNL = max(epsilon, dot(n,l));
	float clamped_dotNV = max(epsilon, dot(n,v));

	float d = ndf_ggx_isotropic(roughness, dot(n,h));
	float g = gaf_schlick_ggx(roughness, clamped_dotNL, clamped_dotNV);
	vec3 f = mix(c_spec, vec3(1), fresnel_schlick(dot(l,h)));

	return max(vec3(0), d * g * f / 4.f);
}

vec3 cook_torrance_ansi_brdf(vec3 n,
							 vec3 t,
							 vec3 b,
							 vec3 v,
							 vec3 l,
							 vec3 h,
							 float roughness_x,
							 float roughness_y,
							 vec3 c_spec) {
	// Like the isotropic case
	float clamped_dotNL = max(epsilon, dot(n,l));
	float clamped_dotNV = max(epsilon, dot(n,v));

	float roughness = mix(roughness_y, roughness_x, dot(t,h));

	float d = ndf_ggx_ansiotropic(t, b, h, roughness_x, roughness_y, dot(n,h));
	float g = gaf_schlick_ggx(roughness, clamped_dotNL, clamped_dotNV);
	vec3 f = mix(c_spec, vec3(1), fresnel_schlick(dot(l,h)));

	return max(vec3(0), d * g * f / 4.f);
}
