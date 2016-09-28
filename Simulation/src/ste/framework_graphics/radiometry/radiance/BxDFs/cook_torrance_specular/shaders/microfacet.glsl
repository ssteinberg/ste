
#include "common.glsl"

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

float ggx_refraction_ratio(sampler2DArray microfacet_refraction_ratio_fit_lut,
						   vec3 v,
						   vec3 n,
						   float roughness,
						   float ior1,
						   float ior2) {
	const float min_ior_ratio = 1.f / 3.f;

	float ior_ratio = clamp(((ior2 / ior1) - min_ior_ratio) / (1f - min_ior_ratio), .0f, 1.f);
	vec2 uv = vec2(ior_ratio, roughness);
	
	vec4 theta = vec4(acos(dot(v, n)));
	
	vec4 a0 = textureGather(microfacet_refraction_ratio_fit_lut, vec3(uv, 0), 0);
	vec4 b0 = textureGather(microfacet_refraction_ratio_fit_lut, vec3(uv, 0), 1);
	vec4 c0 = textureGather(microfacet_refraction_ratio_fit_lut, vec3(uv, 0), 2);
	vec4 a1 = textureGather(microfacet_refraction_ratio_fit_lut, vec3(uv, 1), 0);
	vec4 b1 = textureGather(microfacet_refraction_ratio_fit_lut, vec3(uv, 1), 1);
	vec4 c1 = textureGather(microfacet_refraction_ratio_fit_lut, vec3(uv, 1), 2);
	
	vec4 x0 = (theta - b0) / c0;
	vec4 x1 = (theta - b1) / c1;
	
	vec4 gauss0 = a0 * exp(-x0*x0);
	vec4 gauss1 = a1 * exp(-x1*x1);
	
	vec2 lut_size = textureSize(microfacet_refraction_ratio_fit_lut, 0).xy;
	vec2 w = fract(uv * lut_size - vec2(.5f + 1.f / 4096.f));
	
	vec2 gauss_xy = mix(vec2(gauss0.x, gauss1.x), vec2(gauss0.y, gauss1.y), w.x);
	vec2 gauss_wz = mix(vec2(gauss0.w, gauss1.z), vec2(gauss0.w, gauss1.z), w.x);
	vec2 gauss_lin = mix(gauss_xy, gauss_wz, w.y);

	return gauss_lin.x + gauss_lin.y;
}
