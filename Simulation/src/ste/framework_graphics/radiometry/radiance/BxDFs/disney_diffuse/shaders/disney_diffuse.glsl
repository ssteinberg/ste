
#include "common.glsl"
#include "fresnel.glsl"
#include "microfacet.glsl"

float disney_diffuse_brdf(vec3 n,
						  vec3 v,
						  vec3 l,
						  vec3 h,
						  float roughness) {
	float dotLH = dot(l,h);

	float fresnel_l = fresnel_schlick_ratio(dot(n,l));
	float fresnel_v = fresnel_schlick_ratio(dot(n,v));
	float Fd90 = .5f + 2.f * dotLH*dotLH * roughness;
	return mix(1.f, Fd90, fresnel_l) * mix(1.f, Fd90, fresnel_v) * one_over_pi;
}
