
#include "common.glsl"

float ndf_ggx_isotropic(float roughness, float dotNH) {
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float dotNH2 = dotNH * dotNH;
	float denom_ndf = dotNH2 * (alpha2 - 1.f) + 1.f;

	return alpha2 / (pi * denom_ndf * denom_ndf);
}

float ndf_ggx_ansiotropic(vec3 X, vec3 Y, vec3 h, float roughness_x, float roughness_y, float dotNH) {
	float alphax = roughness_x * roughness_x;
	float alphay = roughness_y * roughness_y;

	float a = pi * alphax * alphay;

	float ansix = dot(X, h) / alphax;
	float ansiy = dot(Y, h) / alphay;

	float denom = ansix * ansix + ansiy * ansiy + dotNH * dotNH;

	return 1.f / (a * denom * denom);
}

float gaf_schlick_ggx(float roughness, float dotNL, float dotNV) {
	float alpha = roughness * roughness;
	float k = alpha / 2.f;
	float invk = 1.f - k;
	float g1 = 1.f / (dotNL * invk + k);
	float g2 = 1.f / (dotNV * invk + k);

	return g1 * g2;
}

float fresnel_schlick(float dotLH) {
	float p = 1.f - dotLH;
	float p2 = p*p;

	return p2 * p2 * p;
}

float fresnel_schlick(float dotLH, float power) {
	float p = 1.f - dotLH;
	return pow(p, power);
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
