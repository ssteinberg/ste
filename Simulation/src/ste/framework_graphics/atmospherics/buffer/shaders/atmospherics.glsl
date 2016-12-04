
#include "atmospherics_descriptor.glsl"

layout(std430, binding = 22) restrict readonly buffer atmospherics_descriptor_buffer {
	atmospherics_descriptor atmospherics_descriptor_data;
};


/*
*	Returns the atmospheric density at world position in kg/m^3
*
*	@param w_pos	World position
*/
float atmospherics_air_density(vec3 w_pos) {
	// At the moment, just use y as height, with 0 being sea level.
	float altitude = max(0, w_pos.y);

	return atmospherics_descriptor_density_at_altitude(atmospherics_descriptor_data, altitude);
}

/*
*	Returns the total extinction coefficient in m^-1
*/
vec3 atmospherics_extinction_coeffcient() {
	vec3 mie = vec3(atmospherics_descriptor_data.mie_scattering_coefficient + atmospherics_descriptor_data.mie_absorption_coefficient);
	return atmospherics_descriptor_data.rayleigh_scattering_coefficient + mie;
}

/*
*	Returns the total atmospheric attenuation coefficient at world position
*	The attenuation coefficient is total extinction coefficient multiplied by air density, and can be used e.g. for Beer-Lambert attenuation.
*/
vec3 atmospherics_attenuation_coeffcient(vec3 w_pos) {
	return atmospherics_air_density(w_pos) * atmospherics_extinction_coeffcient();
}
