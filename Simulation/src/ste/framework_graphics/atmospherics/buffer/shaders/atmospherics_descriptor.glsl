
#include <constants.glsl>

const float atmospherics_optical_length_air_lut_idx = 0;
const float atmospherics_optical_length_aerosols_lut_idx = 1;


// Helper functions for accessing precomputed atmospherics scatter LUT
float _atmospheric_height_to_lut_idx(float h, float h_max) {
	float x = max(.0f, h / h_max);
	return sqrt(x);
}
float _atmospheric_view_zenith_to_lut_idx(float cos_phi) {
	return (1.f + cos_phi) / 2.f;
}
float _atmospheric_sun_zenith_to_lut_idx(float cos_delta) {
	float t = -2.8f * cos_delta - .8f;
	return (1.f - exp(t)) / (1.f - exp(-3.6f));
}
float _atmospheric_sun_view_azimuth_to_lut_idx(float omega) {
	return omega / pi;
}
float _atmospheric_ambient_NdotL_to_lut_idx(float NdotL) {
	return (1.f + NdotL) / 2.f;
}


struct atmospherics_descriptor {
	// Atmosphere center in world coordinates and radius
	vec4 center_radius;

	// xyz: Wave length dependent scattering coefficients for the Rayleigh scattering theory (m^-1)
	// w: Scattering coefficient for the Mie scattering theory (m^-1)
	vec4 scattering_coefficients;
	// Absorption coefficient for the Mie scattering theory (m^-1)
	float mie_absorption_coefficient;
	// Phase coefficient for the Mie scattering phase function
	float phase;

	// Scale heights for Mie and Rayleigh scattering, repectively
	float Hm;
	float Hr;

	// Precomputations
	float minus_one_over_Hm;
	float minus_one_over_Hr;
	
	float Hm_max;
	float Hr_max;
};


float atmospherics_mie_scattering(atmospherics_descriptor desc) {
	return desc.scattering_coefficients.w;
}

float atmospherics_mie_absorption(atmospherics_descriptor desc) {
	return desc.mie_absorption_coefficient;
}

vec3 atmospherics_rayleigh_scattering(atmospherics_descriptor desc) {
	return desc.scattering_coefficients.xyz;
}

float atmospherics_descriptor_pressure(float h, float minus_one_over_H) {
	return exp(minus_one_over_H * h);
}


/*
*	Returns the pressure at h using the exponential barometric law
*
*	@param h	Height in meters
*/
float atmospherics_descriptor_pressure_rayleigh(atmospherics_descriptor desc, float h) {
	return atmospherics_descriptor_pressure(h, desc.minus_one_over_Hr);
}

/*
*	Returns the aerosols pressure at h using the exponential barometric law
*
*	@param h	Height in meters
*/
float atmospherics_descriptor_pressure_mie(atmospherics_descriptor desc, float h) {
	return atmospherics_descriptor_pressure(h, desc.minus_one_over_Hm);
}


/*
*	Returns the total Mie extinction coefficient in m^-1
*/
float atmospherics_descriptor_mie_extinction_coeffcient(atmospherics_descriptor desc) {
	return atmospherics_mie_scattering(desc) + atmospherics_mie_absorption(desc);
}

/*
*	Returns the total Rayleigh extinction coefficient in m^-1
*/
vec3 atmospherics_descriptor_rayleigh_extinction_coeffcient(atmospherics_descriptor desc) {
	return atmospherics_rayleigh_scattering(desc);
}

/*
*	Returns the optical length of a light ray originating at P0 in direction V.
*	Computed using a LUT with precomputed numerical integrated solutions to the integral of the density function.
*
*	@param P0	Start point
*	@param V	Normalized ray direction
*	@param H	Scale height
*	@param C	Atmosphere's (planet) center
*	@param r	Atmosphere's lower radius (i.e. planet radius)
*	@param Hmax	Height limit
*/
float atmospherics_descriptor_optical_length_ray(vec3 P0, vec3 V, float H, 
												 vec3 C, float r,
												 float Hmax,
												 float lut_array_idx,
												 sampler2DArray atmospheric_optical_length_lut) {
	vec3 Y = P0 - C;
	float Ylen = length(Y);
	vec3 N = Y / Ylen;
	
	float h = Ylen - r;
	float cos_phi = dot(N, V);
	
	float h_idx = _atmospheric_height_to_lut_idx(h, Hmax);
	float phi_idx = _atmospheric_view_zenith_to_lut_idx(cos_phi);

	return texture(atmospheric_optical_length_lut, vec3(h_idx, phi_idx, lut_array_idx)).x;
}

/*
*	Returns the optical length between 2 points.
*	Optical length expresses the amount of light attenuated from point P0 to P1.
*
*	@param P0	Start point
*	@param P1	End point
*	@param H	Scale height
*	@param C	Atmosphere's (planet) center
*	@param r	Atmosphere's (planet) radius
*	@param Hmax	Height limit
*/
float atmospherics_descriptor_optical_length(vec3 P0, vec3 P1, float H, 
											 vec3 C, float r,
											 float Hmax,
											 float lut_array_idx,
											 sampler2DArray atmospheric_optical_length_lut) {
	if (P0 == P1)
		return .0f;

	vec3 V = normalize(P1 - P0);
	
	float l0 = atmospherics_descriptor_optical_length_ray(P0, V, H,
														  C, r,
														  Hmax,
														  lut_array_idx,
														  atmospheric_optical_length_lut);
	float l1 = atmospherics_descriptor_optical_length_ray(P1, V, H,
														  C, r,
														  Hmax,
														  lut_array_idx,
														  atmospheric_optical_length_lut);

	return abs(l0 - l1);
}

/*
*	Returns the optical length between 2 points.
*	Optical length expresses the amount of light attenuated from point P0 to P1.
*	Fast verion, doesn't use precomputed LUT. Accurate for small distances.
*
*	@param P0	Start point
*	@param P1	End point
*	@param C	Atmosphere's (planet) center
*	@param r	Atmosphere's (planet) radius
*	@param one_over_H	Scale height reciprocal
*/
float atmospherics_descriptor_optical_length_fast(vec3 P0, vec3 P1,
												  vec3 C, float r,
												  float one_over_H) {
	vec3 P = mix(P0, P1, .5f);
	
	vec3 Y = P - C;
	float Ylen = length(Y);	
	float h = Ylen - r;

	return length(P1 - P0) * atmospherics_descriptor_pressure(h, one_over_H);
}

/*
*	Returns the optical length between 2 points for Rayleigh scattering.
*
*	@param P0	Start point
*	@param P1	End point
*/
float atmospherics_descriptor_optical_length_fast_rayleigh(atmospherics_descriptor desc, 
														   vec3 P0, vec3 P1) {
	return atmospherics_descriptor_optical_length_fast(P0, P1, 
													   desc.center_radius.xyz, desc.center_radius.w, 
													   desc.minus_one_over_Hr);
}
/*
*	Returns the optical length between 2 points for Mie scattering.
*
*	@param P0	Start point
*	@param P1	End point
*/
float atmospherics_descriptor_optical_length_fast_mie(atmospherics_descriptor desc, 
													  vec3 P0, vec3 P1) {
	return atmospherics_descriptor_optical_length_fast(P0, P1, 
													   desc.center_radius.xyz, desc.center_radius.w, 
													   desc.minus_one_over_Hm);
}

/*
*	Returns the optical length between 2 points for Rayleigh scattering.
*
*	@param P0	Start point
*	@param P1	End point
*/
float atmospherics_descriptor_optical_length_rayleigh(atmospherics_descriptor desc, 
													  vec3 P0, vec3 P1, 
													  sampler2DArray atmospheric_optical_length_lut) {
	return atmospherics_descriptor_optical_length(P0, P1, desc.Hr, 
												  desc.center_radius.xyz, desc.center_radius.w, 
												  desc.Hr_max, 
												  atmospherics_optical_length_air_lut_idx,
												  atmospheric_optical_length_lut);
}
/*
*	Returns the optical length between 2 points for Mie scattering.
*
*	@param P0	Start point
*	@param P1	End point
*/
float atmospherics_descriptor_optical_length_mie(atmospherics_descriptor desc, 
												 vec3 P0, vec3 P1,
												 sampler2DArray atmospheric_optical_length_lut) {
	return atmospherics_descriptor_optical_length(P0, P1, desc.Hm, 
												  desc.center_radius.xyz, desc.center_radius.w, 
												  desc.Hm_max, 
												  atmospherics_optical_length_aerosols_lut_idx,
												  atmospheric_optical_length_lut);
}

/*
*	Returns the optical length for a ray for Rayleigh scattering.
*
*	@param P0	Start point
*	@param V	Ray direction
*/
float atmospherics_descriptor_optical_length_ray_rayleigh(atmospherics_descriptor desc, 
														  vec3 P0, vec3 V,
														  sampler2DArray atmospheric_optical_length_lut) {
	return atmospherics_descriptor_optical_length_ray(P0, V, desc.Hr, 
													  desc.center_radius.xyz, desc.center_radius.w, 
													  desc.Hr_max, 
													  atmospherics_optical_length_air_lut_idx,
													  atmospheric_optical_length_lut);
}
/*
*	Returns the optical length for a ray for Mie scattering.
*
*	@param P0	Start point
*	@param V	Ray direction
*/
float atmospherics_descriptor_optical_length_ray_mie(atmospherics_descriptor desc, 
													 vec3 P0, vec3 V,
													 sampler2DArray atmospheric_optical_length_lut) {
	return atmospherics_descriptor_optical_length_ray(P0, V, desc.Hm, 
													  desc.center_radius.xyz, desc.center_radius.w, 
													  desc.Hm_max, 
													  atmospherics_optical_length_aerosols_lut_idx,
													  atmospheric_optical_length_lut);
}
