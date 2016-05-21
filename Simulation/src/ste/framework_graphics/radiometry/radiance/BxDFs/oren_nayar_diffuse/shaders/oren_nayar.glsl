
#include "common.glsl"

float oren_nayar_brdf(vec3 n,
					  vec3 v,
					  vec3 l,
					  float roughness) {
	float roughness2 = roughness * roughness;
	float dotNL = dot(n,l);
	float dotVL = dot(v,l);
	float dotNV = dot(n,v);

	float s = max(0.f, dotVL - dotNV * dotNL);
	float t = min(1.f, dotNL / dotNV);

	float r1 = roughness2 / (roughness2 + 0.33f);
	float a = dotNL * (1.f - r1 * .5f);

	float b = (0.45f * roughness2 / (roughness2 + 0.09f)) * s * t;

	float d = a + b;

	return max(.0f, d / pi);
}
