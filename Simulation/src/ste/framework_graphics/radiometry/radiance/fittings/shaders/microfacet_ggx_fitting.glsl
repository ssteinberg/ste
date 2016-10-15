
#include "common.glsl"

/*
 *	Evaluate Fresnel transmission using precomputed fitting for GGX NDF
 *
 *	@param microfacet_transmission_fit_lut		Look-up-table
 *	@param v			Outbound vector (facing away from fragment to camera)
 *	@param n			Normal
 *	@param roughness	Material roughness
 *	@param sin_critical	Sine of critical angle
 */
float ggx_transmission_ratio(sampler2DArray microfacet_transmission_fit_lut,
							 vec3 v,
							 vec3 n,
							 float roughness,
							 float sin_critical) {
	const float Rmin = .2;
	const float Rmax = 3.2;
	
	float x = dot(v, n);
	float ior_ratio = clamp((sin_critical - Rmin) / (Rmax - Rmin), .0f, 1.f);
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
 *	Evaluate refracted vector using precomputed fitting for GGX NDF
 *
 *	@param microfacet_refraction_fit_lut		Look-up-table
 *	@param v			Outbound vector (facing away from fragment to camera)
 *	@param n			Normal
 *	@param roughness	Material roughness
 *	@param sin_critical	Sine of critical angle
 */
vec3 ggx_refract(sampler2D microfacet_refraction_fit_lut,
				 vec3 v,
				 vec3 n,
				 float roughness,
				 float sin_critical) {
	const float Rmin = .2;
	const float Rmax = 3.2;
	
	float x = dot(v, n);
	float ior_ratio = clamp((sin_critical - Rmin) / (Rmax - Rmin), .0f, 1.f);
	vec2 uv = vec2(ior_ratio, roughness);
	
	vec4 lut = texture(microfacet_refraction_fit_lut, uv);
	vec2 fit = lut.xz * exp(lut.yw * vec2(x));

	float vx = -clamp(fit.x + fit.y, -1.f, 1.f);
	
	vec3 w = normalize(cross(n,cross(n,v)));
	vec3 t = vx * w - sqrt(1 - vx*vx) * n;

	return t;
}
