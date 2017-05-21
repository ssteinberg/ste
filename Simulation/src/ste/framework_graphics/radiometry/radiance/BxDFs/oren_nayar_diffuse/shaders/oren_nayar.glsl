
#include <common.glsl>

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

	return max(.0f, d * one_over_pi);
}

//
//	Improved Oren-Nayar
//	Energy conserving with Fresnel and GGX for NDF
//
//	Source: Designing Reflectance Models for New Consoles [Gotanda 2014]
//

float improved_oren_nayar_Lm(float r,
							 float dotNL) {
	float one_m_NdotL = 1.f - dotNL;
	float one_m_NdotL2 = one_m_NdotL * one_m_NdotL;

	float a = max(1.f - 2.f * r, .0f);
	float b = 1.f - one_m_NdotL2 * one_m_NdotL2 * one_m_NdotL;
	float p1 = a * b + min(2*r, 1);

	float p2 = dotNL * (1.f - .5f * r) + .5f * r * dotNL * dotNL;

	return p1 * p2;
}

float improved_oren_nayar_Vd(float r2,
							 float dotNL,
							 float dotNV) {
	float power = (1.f - 0.3726732f * dotNV * dotNV) / (.188566f + .388410 * dotNV);
	float p1 = r2 / ((r2 + .09f) * (1.31072f + .995584f * dotNV));
	float p2 = 1.f - pow(1.f - dotNL, power);

	return p1 * p2;
}

float improved_oren_nayar_Bp(float dotNL,
							 float dotNV,
							 float dotVL) {
	float delta = dotVL - dotNV * dotNL;
	if (delta < .0f)
		return 1.4f * dotNV * dotNL * delta;
	else
		return delta;
}

float improved_oren_nayar_Fr(float r,
							 float r2,
							 float dotNV) {
	float r3 = r2 * r;
	float r4 = r2 * r2;

	float p1 = 1.f - (.542026f * r2 + .303573 * r) / (r2 + 1.36053f);
	float p2 = 1.f - pow(1 - dotNV, 5.f - 4.f * r2);
	float p3 = (-.733996f*r3 + 1.50912*r2 - 1.16402*r) * pow(1 - dotNV, 1.f + 1.f / (39.f * r4 + 1.f)) + 1.f;

	return p1 * p2 * p3;
}

vec3 improved_oren_nayar_brdf(vec3 n,
							  vec3 v,
							  vec3 l,
							  float roughness,
							  vec3 f0) {
	float roughness2 = roughness * roughness;
	float dotNL = dot(n,l);
	float dotVL = dot(v,l);
	float dotNV = dot(n,v);

	float Lm = improved_oren_nayar_Lm(roughness, dotNL);
	float Vd = improved_oren_nayar_Vd(roughness2, dotNL, dotNV);
	float Bp = improved_oren_nayar_Bp(dotNL, dotNV, dotVL);
	float Fr = improved_oren_nayar_Fr(roughness, roughness2, dotNV);

	return 21.f / (20.f * pi) * (vec3(1) - f0) * (Fr * Lm + Vd * Bp);
}
