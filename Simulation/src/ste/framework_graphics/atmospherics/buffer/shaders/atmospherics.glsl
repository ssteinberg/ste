
#include "atmospherics_descriptor.glsl"
#include "light_transport.glsl"

layout(std430, binding = 22) restrict readonly buffer atmospherics_descriptor_buffer {
	atmospherics_descriptor atmospherics_descriptor_data;
};


/*
*	Returns the atmospheric air density at world position in kg/m^3
*
*	@param w_pos	World position
*/
float atmospherics_air_density(vec3 w_pos) {
	// At the moment, just use y as height, with 0 being sea level.
	float altitude = max(0, w_pos.y);
	return atmospherics_descriptor_pressure_rayleigh(atmospherics_descriptor_data, altitude);
}

/*
*	Returns the atmospheric aerosols density at world position in kg/m^3
*
*	@param w_pos	World position
*/
float atmospherics_aerosol_density(vec3 w_pos) {
	// At the moment, just use y as height, with 0 being sea level.
	float altitude = max(0, w_pos.y);
	return atmospherics_descriptor_pressure_mie(atmospherics_descriptor_data, altitude);
}

/*
*	Returns the atmospheric air optical length
*
*	@param P0	Start world position
*	@param P1	End world position
*/
float atmospherics_optical_length_air(vec3 P0, vec3 P1) {
	return atmospherics_descriptor_optical_length_rayleigh(atmospherics_descriptor_data, P0, P1);
}

/*
*	Returns the atmospheric aerosol optical length
*
*	@param P0	Start world position
*	@param P1	End world position
*/
float atmospherics_optical_length_aerosol(vec3 P0, vec3 P1) {
	return atmospherics_descriptor_optical_length_mie(atmospherics_descriptor_data, P0, P1);
}

/*
*	Returns the atmospheric air optical length for path originating at infinite height in direction 
*	V and ending at P1.
*
*	@param P1	End world position
*	@param V	Normalized ray direction
*/
float atmospherics_optical_length_from_infinity_air(vec3 P1, vec3 V) {
	return atmospherics_descriptor_optical_length_from_infinity_rayleigh(atmospherics_descriptor_data, P1, V);
}

/*
*	Returns the atmospheric aerosol optical length for path originating at infinite height in direction 
*	V and ending at P1.
*
*	@param P1	End world position
*	@param V	Normalized ray direction
*/
float atmospherics_optical_length_from_infinity_aerosol(vec3 P1, vec3 V) {
	return atmospherics_descriptor_optical_length_from_infinity_mie(atmospherics_descriptor_data, P1, V);
}

/*
*	Returns the Mie extinction coefficient in m^-1
*/
float atmospherics_mie_extinction_coeffcient() {
	return atmospherics_descriptor_data.mie_scattering_coefficient + atmospherics_descriptor_data.mie_absorption_coefficient;
}

/*
*	Returns the Mie extinction coefficient in m^-1
*/
vec3 atmospherics_rayleigh_extinction_coeffcient() {
	return atmospherics_descriptor_data.rayleigh_scattering_coefficient;
}

/*
*	Calculates the total atmospheric extinction along a path.
*
*	@param P0	Start world position
*	@param P1	End world position
*/
vec3 extinct(vec3 P0, vec3 P1) {
	float tr = atmospherics_optical_length_air(P0, P1);
	float tm = atmospherics_optical_length_aerosol(P0, P1);
	vec3 t = atmospherics_rayleigh_extinction_coeffcient() * tr +
			 vec3(atmospherics_mie_extinction_coeffcient()) * tm;
	return beer_lambert(t);
}
/*
*	Calculates the total atmospheric extinction along a multi-point route.
*	The route is the lines (P0,P1) and (P1,P2).
*
*	@param P0	World position
*	@param P1	World position
*	@param P2	World position
*/
vec3 extinct(vec3 P0, vec3 P1, vec3 P2) {
	float tr = atmospherics_optical_length_air(P0, P1) + 
			   atmospherics_optical_length_air(P1, P2);
	float tm = atmospherics_optical_length_aerosol(P0, P1) +
			   atmospherics_optical_length_aerosol(P1, P2);
	vec3 t = atmospherics_rayleigh_extinction_coeffcient() * tr +
			 vec3(atmospherics_mie_extinction_coeffcient()) * tm;
	return beer_lambert(t);
}
/*
*	Calculates the total atmospheric extinction along a multi-point route.
*	The route is the lines (P0,P1), (P1,P2), etc.
*
*	@param P0	World position
*	@param P1	World position
*	@param P2	World position
*	@param P3	World position
*/
vec3 extinct(vec3 P0, vec3 P1, vec3 P2, vec3 P3) {
	float tr = atmospherics_optical_length_air(P0, P1) + 
			   atmospherics_optical_length_air(P1, P2) + 
			   atmospherics_optical_length_air(P2, P3);
	float tm = atmospherics_optical_length_aerosol(P0, P1) +
			   atmospherics_optical_length_aerosol(P1, P2) +
			   atmospherics_optical_length_aerosol(P2, P3);
	vec3 t = atmospherics_rayleigh_extinction_coeffcient() * tr +
			 vec3(atmospherics_mie_extinction_coeffcient()) * tm;
	return beer_lambert(t);
}

/*
*	Calculates the total atmospheric extinction along a path orignating at infinite height.
*
*	@param P1	End point in world coordinates
*	@param V	Normalized ray direction in world coordinates
*/
vec3 extinct_from_infinity(vec3 P1, vec3 V) {
	float tr = atmospherics_optical_length_from_infinity_air(P1, V);
	float tm = atmospherics_optical_length_from_infinity_aerosol(P1, V);
	vec3 t = atmospherics_rayleigh_extinction_coeffcient() * tr +
			 vec3(atmospherics_mie_extinction_coeffcient()) * tm;
	return beer_lambert(t);
}

/*
*	Calculates the total atmospheric extinction along a path orignating at infinite height.
*	The route is the ray from infinity to P1 in direction V and the line (P1, P2).
*
*	@param P1	World position
*	@param V	Normalized ray direction in world coordinates
*	@param P2	World position
*/
vec3 extinct_from_infinity(vec3 P1, vec3 V, vec3 P2) {
	float tr = atmospherics_optical_length_from_infinity_air(P1, V) + 
			   atmospherics_optical_length_air(P1, P2);
	float tm = atmospherics_optical_length_from_infinity_aerosol(P1, V) +
			   atmospherics_optical_length_aerosol(P1, P2);
	vec3 t = atmospherics_rayleigh_extinction_coeffcient() * tr +
			 vec3(atmospherics_mie_extinction_coeffcient()) * tm;
	return beer_lambert(t);
}

/*
*	Calculates a single atmospheric scattering event and the total atmospheric extinction along a path.
*
*	@param P0		Start world position
*	@param P1		Scatter point, as world position
*	@param P2		End world position
*	@param I		Precomputed direction of light incident ray, originating from P1 to P2, normalized.
*	@param L		Precomputed direction of light incident ray, originating from P1 to P0, normalized.
*	@param volume	Volume, in meters^3, of the scattering event area
*/
vec3 scatter(vec3 P0, 
			 vec3 P1, 
			 vec3 P2, 
			 vec3 I, 
			 vec3 L,
			 float volume) {	
	float tr = atmospherics_optical_length_air(P0, P1) + 
			   atmospherics_optical_length_air(P1, P2);
	float tm = atmospherics_optical_length_aerosol(P0, P1) +
			   atmospherics_optical_length_aerosol(P1, P2);

	vec3 i = I;
	vec3 o = L;
	float p_mie = cornette_shanks_phase_function(i, o, atmospherics_descriptor_data.phase);
	float p_rayleigh = rayleigh_phase_function(i, o);
	vec3 scatter_coefficient = beer_lambert(tm) * p_mie * atmospherics_descriptor_data.mie_scattering_coefficient.xxx + 
							   beer_lambert(tr) * p_rayleigh * atmospherics_descriptor_data.rayleigh_scattering_coefficient;
							   
	float particle_density = atmospherics_air_density(P1);

	return scatter_coefficient * volume * particle_density;
}
/*
*	Calculates a single atmospheric scattering event and the total atmospheric extinction along a path.
*
*	@param P0		Start world position
*	@param P1		Scatter point, as world position
*	@param P2		End world position
*	@param volume	Volume, in meters^3, of the scattering event area
*/
vec3 scatter(vec3 P0, 
			 vec3 P1, 
			 vec3 P2, 
			 float volume) {
	vec3 I = normalize(P2 - P1);
	vec3 L = normalize(P0 - P1);
	return scatter(P0,
				   P1,
				   P2,
				   I, L,
				   volume);
}

/*
*	Calculates a single atmospheric scattering event and the total atmospheric extinction along a path orignating 
*	at infinite height
*
*	@param P1		Scatter point, as world position
*	@param V		Normalized ray direction in world coordinates
*	@param P2		End world position
*	@param I		Direction of light incident ray, originating from P1 to P2, normalized.
*	@param volume	Volume, in meters^3, of the scattering event area
*/
vec3 scatter_from_infinity(vec3 P1, 
						   vec3 V, 
						   vec3 P2,
						   vec3 I, 
						   float volume) {
	float tr = atmospherics_optical_length_from_infinity_air(P1, V) + 
			   atmospherics_optical_length_air(P1, P2);
	float tm = atmospherics_optical_length_from_infinity_aerosol(P1, V) +
			   atmospherics_optical_length_aerosol(P1, P2);

	vec3 i = I;
	vec3 o = V;
	float p_mie = cornette_shanks_phase_function(i, o, atmospherics_descriptor_data.phase);
	float p_rayleigh = rayleigh_phase_function(i, o);
	vec3 scatter_coefficient = beer_lambert(tm) * p_mie * atmospherics_descriptor_data.mie_scattering_coefficient.xxx + 
							   beer_lambert(tr) * p_rayleigh * atmospherics_descriptor_data.rayleigh_scattering_coefficient;
							   
	float particle_density = atmospherics_air_density(P1);
	
	return scatter_coefficient * volume * particle_density;
}
