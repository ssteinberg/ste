
#include "common.glsl"

float material_anisotropic_roughness(float rx, float ry,
									 vec3 n, vec3 t,
									 vec3 v) {
	vec3 w = v - dot(v, n)*n;
	float lenw = length(w);
	float ansi_ratio = lenw == .0f ? 
							.5f : 
							abs(dot(w / lenw, t));

	return mix(ry, rx, ansi_ratio);
}

float ndf_ggx_isotropic_cdf(float roughness, float dotNH) {
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float dotNH2 = dotNH * dotNH;
	float t = alpha2 - 1.f;

	float denom = t * (dotNH2 * t + 1.f);

	return alpha2 / denom - 1.f / t;
}

float ndf_ggx_isotropic(float roughness, float dotNH) {
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float dotNH2 = dotNH * dotNH;
	float denom_ndf = dotNH2 * (alpha2 - 1.f) + 1.f;

	return one_over_pi * alpha2 / (denom_ndf * denom_ndf);
}

float ndf_ggx_ansiotropic(vec3 X, vec3 Y, vec3 h, float roughness_x, float roughness_y, float dotNH) {
	float alphax = roughness_x * roughness_x;
	float alphay = roughness_y * roughness_y;

	float a = alphax * alphay;

	float ansix = dot(X, h) / alphax;
	float ansiy = dot(Y, h) / alphay;

	float denom = ansix * ansix + ansiy * ansiy + dotNH * dotNH;

	return one_over_pi / (a * denom * denom);
}

float gaf_schlick_ggx(float roughness, float dotNL, float dotNV, out float g_mask, out float g_shadow) {
	float alpha = roughness * roughness;
	float k = alpha / 2.f;
	float invk = 1.f - k;

	g_mask = 1.f / (dotNL * invk + k);
	g_shadow = 1.f / (dotNV * invk + k);

	return g_mask * g_shadow;
}

float fresnel_schlick_ratio(float dotLH) {
	float p = 1.f - dotLH;
	float p2 = p*p;
	return p2 * p2 * p;
}

float fresnel_schlick_ratio(float dotLH, float power) {
	float p = 1.f - dotLH;
	return pow(p, power);
}

float fresnel_schlick(float F0, float dotLH) {
	return mix(F0, 1.f, fresnel_schlick_ratio(dotLH));
}

float fresnel_schlick_tir(float F0, float dotLH, float cos_critical) {
	if (dotLH <= cos_critical)
		return 1.f;

	float p = 1.f - (dotLH - cos_critical) / (1 - cos_critical);
	float p2 = p*p;
	float a = p2 * p2;

	return mix(F0, 1.f, a);
}

vec3 fresnel_cook_torrance(float dotLH, vec3 F0) {
	vec3 sqrtF0 = sqrt(F0);
	vec3 eta = (vec3(1) + sqrtF0) / (vec3(1) - sqrtF0);
	vec3 gamma = sqrt(eta * eta + vec3(dotLH * dotLH) - vec3(1));

	vec3 gpc = gamma + vec3(dotLH);
	vec3 gmc = gamma - vec3(dotLH);

	vec3 f1 = gmc / gpc;
	vec3 f2 = (gpc * dotLH - vec3(1)) / (gmc * dotLH + vec3(1));

	return .5f * f1 * f1 * (vec3(1) + f2 * f2);
}
