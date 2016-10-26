
#include "common.glsl"
#include "erf.glsl"

/*
 *	Evaluate Fresnel transmission using precomputed fitting for GGX NDF (v3)
 *
 *	@param microfacet_transmission_fit_lut		Look-up-table
 *	@param v			Outbound vector (facing away from fragment to camera)
 *	@param n			Normal
 *	@param roughness	Material roughness
 *	@param refractive_ratio	Ratio of refractive-indices, ior2/ior1
 */
float ggx_transmission_ratio_v3(sampler2DArray microfacet_transmission_fit_lut,
								vec3 v,
								vec3 n,
								float roughness,
								float refractive_ratio) {
	const float Rmin = .2;
	const float Rmax = 3.2;
	
	float x = dot(v, n);
	float ior_ratio = clamp((refractive_ratio - Rmin) / (Rmax - Rmin), .0f, 1.f);
	vec2 uv = vec2(ior_ratio, roughness);
	
	vec3 lut0 = texture(microfacet_transmission_fit_lut, vec3(uv, 0)).xyz;
	vec3 lut1 = texture(microfacet_transmission_fit_lut, vec3(uv, 1)).xyz;

	vec2 a = lut0.xy;
	vec2 b = vec2(lut0.z, lut1.z);
	vec2 c = lut1.xy;
	
	vec2 t = (vec2(x) + b) * c;
	vec2 gauss = a * exp(-(t * t));

	return clamp(gauss.x + gauss.y, .0f, 1.f);
}

/*
 *	Evaluate Fresnel transmission using precomputed fitting for GGX NDF (v4)
 *
 *	@param microfacet_transmission_fit_lut		Look-up-table
 *	@param v			Outbound vector (facing away from fragment to camera)
 *	@param n			Normal
 *	@param roughness	Material roughness
 *	@param refractive_ratio	Ratio of refractive-indices, ior2/ior1
 */
float ggx_transmission_ratio_v4(sampler2DArray microfacet_transmission_fit_lut,
								vec3 v,
								vec3 n,
								float roughness,
								float refractive_ratio) {
	const float Rmin = .2;
	const float Rmax = 3.2;
	
	float x = dot(v, n);
	float ior_ratio = clamp((refractive_ratio - Rmin) / (Rmax - Rmin), .0f, 1.f);
	vec2 uv = vec2(ior_ratio, roughness);
	
	vec3 lut0 = texture(microfacet_transmission_fit_lut, vec3(uv, 0)).xyz;
	vec3 lut1 = texture(microfacet_transmission_fit_lut, vec3(uv, 1)).xyz;
	float a = lut0.x;
	float b = lut0.y;
	float c = lut0.z;
	float d = lut1.x;
	float m = lut1.y;

	return clamp(m*((erf(a*x - b) + 1.f) + c*x) + d, .0f, 1.f);
}

/*
 *	Evaluate refracted vector using precomputed fitting for GGX NDF
 *
 *	@param microfacet_refraction_fit_lut		Look-up-table
 *	@param v			Outbound vector (facing away from fragment to camera)
 *	@param n			Normal
 *	@param roughness	Material roughness
 *	@param refractive_ratio	Ratio of refractive-indices, ior2/ior1
 */
vec3 ggx_refract(sampler2D microfacet_refraction_fit_lut,
				 vec3 v,
				 vec3 n,
				 float roughness,
				 float refractive_ratio) {
	const float Rmin = .2;
	const float Rmax = 3.2;
	
	float x = dot(v, n);
	float ior_ratio = clamp((refractive_ratio - Rmin) / (Rmax - Rmin), .0f, 1.f);
	vec2 uv = vec2(ior_ratio, roughness);
	
	vec4 lut = texture(microfacet_refraction_fit_lut, uv);
	vec2 fit = lut.xz * exp(lut.yw * vec2(x));

	float vx = -clamp(fit.x + fit.y, -1.f, 1.f);
	
	vec3 w = normalize(cross(n,cross(n,v)));
	vec3 t = vx * w - sqrt(1 - vx*vx) * n;

	return t;
}
