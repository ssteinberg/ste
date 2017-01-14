
#include "common.glsl"

/*
 *	The Henyey-Greenstein phase function. 
 *	Usually used for Mie theory scattering.
 *
 *	@param cosine	Cosine of input angle.
 *	@param g		Scattering variation parameter. 
 *					Ranges from backscattering (<0) through isotropic scattering (0) to forward scattering (>0).  (-1,+1)
 */
float henyey_greenstein_phase_function(float cosine, float g) {
	float g2 = g*g;
	float denom = four_pi * pow(1.f + g2 - 2.f * g * cosine, 3.f / 2.f);

	return (1.f - g2) / denom;
}
float henyey_greenstein_phase_function(vec3 u, vec3 v, float g) {
	return henyey_greenstein_phase_function(dot(u, v), g);
}

/*
 *	The Cornette-Shanks phase function, an updated Henyey-Greenstein phase function.
 *
 *	@param cosine	Cosine of input angle.
 *	@param g		Scattering variation parameter. 
 *					Ranges from backscattering (<0) through isotropic scattering (0) to forward scattering (>0).  (-1,+1)
 */
float cornette_shanks_phase_function(float cosine, float g) {
	float g2 = g*g;
	float denom = 2.f * four_pi * (2.f + g2) * pow(1.f + g2 - 2.f * g * cosine, 3.f / 2.f);

	return 3.f * (1.f + cosine*cosine) * (1.f - g2) / denom;
}
float cornette_shanks_phase_function(vec3 u, vec3 v, float g) {
	return cornette_shanks_phase_function(dot(u, v), g);
}

/*
 *	The Rayleigh (normalized) phase function.
 *
 *	@param cosine	Cosine of input angle.
 */
float rayleigh_phase_function(float cosine) {
	return 3.f / (4.f * four_pi) * (1.f + cosine * cosine);
}
float rayleigh_phase_function(vec3 u, vec3 v) {
	return rayleigh_phase_function(dot(u, v));
}

/*
 *	The Beer-Lambert law relates the attenuation of light to the properties of the material through which the light is traveling.
 *
 *	@param att			Attenuation coefficients per channel.  [0, inf]
 *	@param path_length	Path length.  [0, inf]
 */
float beer_lambert(float att, float path_length) {
	return exp(-max(flt_min, path_length) * att);
}
vec2 beer_lambert(vec2 att, float path_length) {
	return exp(-max(flt_min, path_length) * att);
}
vec3 beer_lambert(vec3 att, float path_length) {
	return exp(-max(flt_min, path_length) * att);
}
vec4 beer_lambert(vec4 att, float path_length) {
	return exp(-max(flt_min, path_length) * att);
}
float beer_lambert(float att) {
	return exp(-att);
}
vec2 beer_lambert(vec2 att) {
	return exp(-att);
}
vec3 beer_lambert(vec3 att) {
	return exp(-att);
}
vec4 beer_lambert(vec4 att) {
	return exp(-att);
}
