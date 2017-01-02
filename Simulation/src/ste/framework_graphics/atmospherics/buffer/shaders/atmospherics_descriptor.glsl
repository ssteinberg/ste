
#include "constants.glsl"

struct atmospherics_descriptor {
	// Wave length dependent scattering coefficients for the Rayleigh scattering theory (m^-1)
	vec3 rayleigh_scattering_coefficient;
	// Scattering coefficient for the Mie scattering theory (m^-1)
	float mie_scattering_coefficient;
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

	float _unused[2];
};


/*
*	Returns the pressure at h using the exponential barometric law
*
*	@param h	Height in meters
*/
float atmospherics_descriptor_pressure_rayleigh(atmospherics_descriptor desc, float h) {
	return exp(desc.minus_one_over_Hr * h);
}

/*
*	Returns the aerosols pressure at h using the exponential barometric law
*
*	@param h	Height in meters
*/
float atmospherics_descriptor_pressure_mie(atmospherics_descriptor desc, float h) {
	return exp(desc.minus_one_over_Hm * h);
}


/*
*	Returns the total Mie extinction coefficient in m^-1
*/
float atmospherics_descriptor_mie_extinction_coeffcient(atmospherics_descriptor desc) {
	return desc.mie_scattering_coefficient + desc.mie_absorption_coefficient;
}

/*
*	Returns the total Rayleigh extinction coefficient in m^-1
*/
vec3 atmospherics_descriptor_rayleigh_extinction_coeffcient(atmospherics_descriptor desc) {
	return desc.rayleigh_scattering_coefficient;
}

/*
*	Returns the optical length between 2 points.
*	Optical length expresses the amount of light attenuated from point P0 to P1.
*	Computed using an analytical solution to the integral of density_at_altitude(h) from P0 to P1.
*
*	@param P0	Start point
*	@param P1	End point
*	@param H	Scale height
*	@param minus_one_over_H		Precomputed (-1/H)
*/
float atmospherics_descriptor_optical_length(vec3 P0, vec3 P1, float H, float minus_one_over_H) {
	//! Currently the planet is flat...
	float h0 = max(P0.y, .0f);
	float h1 = max(P1.y, .0f);

	float len = length(P1 - P0);
	float climb = abs(h1 - h0);

	if (climb < 1e-2f) {
		float h = mix(h0, h1, .5f);
		return len * exp(minus_one_over_H * h);
	}
	else {
		float a = exp(minus_one_over_H * h0);
		float b = exp(minus_one_over_H * h1);
		return len / climb * H * abs(a - b);
	}
}
float atmospherics_descriptor_optical_length_rayleigh(atmospherics_descriptor desc, vec3 P0, vec3 P1) {
	return atmospherics_descriptor_optical_length(P0, P1, desc.Hr, desc.minus_one_over_Hr);
}
float atmospherics_descriptor_optical_length_mie(atmospherics_descriptor desc, vec3 P0, vec3 P1) {
	return atmospherics_descriptor_optical_length(P0, P1, desc.Hm, desc.minus_one_over_Hm);
}

/*
*	Returns the optical length of a light ray originating at infinite height in direction 
*	V and ending at P1.
*	For more details see atmospherics_descriptor_optical_length().
*
*	@param P1	End point
*	@param V	Normalized ray direction
*	@param H	Scale height
*	@param minus_one_over_H		Precomputed (-1/H)
*/
float atmospherics_descriptor_optical_length_from_infinity(vec3 P1, vec3 V, float H, float minus_one_over_H) {
	vec3 up = vec3(0,1,0);
	float h1 = max(P1.y, .0f);

	float delta = dot(up, -V);
	// Ignore rays coming from below the ground
	if (delta <= 0)
		return +inf;

	float normalizer = 1.f / delta;

	return normalizer * H * exp(minus_one_over_H * h1);
}
float atmospherics_descriptor_optical_length_from_infinity_rayleigh(atmospherics_descriptor desc, vec3 P1, vec3 V) {
	return atmospherics_descriptor_optical_length_from_infinity(P1, V, desc.Hr, desc.minus_one_over_Hr);
}
float atmospherics_descriptor_optical_length_from_infinity_mie(atmospherics_descriptor desc, vec3 P1, vec3 V) {
	return atmospherics_descriptor_optical_length_from_infinity(P1, V, desc.Hm, desc.minus_one_over_Hm);
}
