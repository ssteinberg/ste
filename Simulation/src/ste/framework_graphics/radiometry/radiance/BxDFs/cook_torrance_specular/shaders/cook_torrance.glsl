
#include "common.glsl"
#include "microfacet.glsl"
#include "fresnel.glsl"

/*
 *	Cook-Torrance isotropic BRDF
 *
 *	@param n			Normal
 *	@param v			Outbound vector (facing away from fragment to camera)
 *	@param l			Incident vector (facing away from fragment to light)
 *	@param h			Half vector
 *	@param roughness	Material roughness
 *	@param cos_critical	Cosine of critical angle
 *	@param sin_critical	Sine of critical angle
 *	@param c_spec		Specular color
 *	@param D			Outputs microfacet NDF value
 *	@param Gmask		Outputs microfacet G1 masking value
 *	@param Gshadow		Outputs microfacet G1 shadowing value
 *	@param F			Outputs Fresnel reflectance value
 */
vec3 cook_torrance_iso_brdf(vec3 n,
							vec3 v,
							vec3 l,
							vec3 h,
							float roughness,
							float cos_critical, float sin_critical,
							vec3 c_spec,
							out float D,
							out float Gmask,
							out float Gshadow,
							out float F) {
	// Singularity at "grazing-angles", i.e. dot(n,v) == 0
	float clamped_dotNL = max(epsilon, dot(n,l));
	float clamped_dotNV = max(epsilon, dot(n,v));

	D = ndf_ggx_isotropic(roughness, dot(n,h));
	float G = gaf_schlick_ggx(roughness, clamped_dotNL, clamped_dotNV, Gmask, Gshadow);
	F = fresnel(dot(l,h), cos_critical, sin_critical);

	return c_spec * max(.0f, D * G * F / 4.f);
}

/*
 *	Cook-Torrance ansiotropic BRDF
 *
 *	@param n			Normal
 *	@param t			Tangent
 *	@param b			Bitangent
 *	@param v			Outbound vector (facing away from fragment to camera)
 *	@param l			Incident vector (facing away from fragment to light)
 *	@param h			Half vector
 *	@param roughness_x	Material roughness in tangent direction
 *	@param roughness_y	Material roughness in bitangent direction
 *	@param cos_critical	Cosine of critical angle
 *	@param sin_critical	Sine of critical angle
 *	@param c_spec		Specular color
 *	@param D			Outputs microfacet NDF value
 *	@param Gmask		Outputs microfacet G1 masking value
 *	@param Gshadow		Outputs microfacet G1 shadowing value
 *	@param F			Outputs Fresnel reflectance value
 */
vec3 cook_torrance_ansi_brdf(vec3 n,
							 vec3 t,
							 vec3 b,
							 vec3 v,
							 vec3 l,
							 vec3 h,
							 float roughness_x,
							 float roughness_y,
							 float cos_critical, float sin_critical,
							 vec3 c_spec,
							 out float D,
							 out float Gmask,
							 out float Gshadow,
							 out float F) {
	// Like the isotropic case
	float clamped_dotNL = max(epsilon, dot(n,l));
	float clamped_dotNV = max(epsilon, dot(n,v));

	float ansi_ratio = clamp(dot(t,h) / (dot(t,h) + dot(b,h)), .0f, 1.f);
	float roughness = mix(roughness_y, roughness_x, ansi_ratio);

	D = ndf_ggx_ansiotropic(t, b, h, roughness_x, roughness_y, dot(n,h));
	float G = gaf_schlick_ggx(roughness, clamped_dotNL, clamped_dotNV, Gmask, Gshadow);
	F = fresnel(dot(l,h), cos_critical, sin_critical);

	return c_spec * max(.0f, D * G * F / 4.f);
}
