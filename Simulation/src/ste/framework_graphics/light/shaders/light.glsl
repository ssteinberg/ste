
#include "common.glsl"

#include "quaternion.glsl"
#include "dual_quaternion.glsl"

#include "girenderer_transform_buffer.glsl"

#include "light_transport.glsl"
#include "atmospherics.glsl"

const int LightTypeSphere = 0;
const int LightTypeDirectional = 1;

const int max_active_lights_per_frame = 24;
const int max_active_directional_lights_per_frame = 4;
const int total_max_active_lights_per_frame = max_active_lights_per_frame + max_active_directional_lights_per_frame;

const float light_minimal_luminance_multiplier = 5e-6f;

struct light_descriptor {
	// position: Light position for spherical, direction for directional lights.
	// Radius is light's radius (all light types)
	// Diffuse is radiance color and luminance is intensity (all light types)
	vec3 position;	float radius;
	vec3 diffuse;	float luminance;

	// Light type
	uint32_t type;
	
	// directional_distance: For directional lights only, distance from origin opposite to lights direction (i.e. -directional_distance*position)
	float directional_distance;
	uint32_t cascade_idx;
	
	//! The rest is used internally only

	// shadow_face_mask: For masking (culling) shadow map cube faces (for spherical) or cascades (for directional).
	uint32_t shadow_face_mask;
	
	// transformed_position: Light position in eye space for spherical, direction in eye space for directional lights.
	vec3 transformed_position;
	float effective_range;
};

/*
 *	Returns light's effective range, i.e. range at which light radiates at least minimal luminance.
 */
float light_effective_range(light_descriptor ld) {
	return ld.effective_range;
}

/*
 *	Transforms light's position/direction based on transformation dual quaternion
 */
vec3 light_transform(dual_quaternion transform, light_descriptor ld) {
	return ld.type == LightTypeDirectional ?
				quat_mul_vec(transform.real, ld.position) :
				dquat_mul_vec(transform, ld.position);
}

/*
 *	Calculates light's effective range given minimal luminance desired
 */
float light_calculate_effective_range(light_descriptor ld, float min_lum) {
	float l = min_lum;
	return ld.radius * (sqrt(ld.luminance / l) - 1.f);
}

/*
 *	Calculates a suggested light's minimal luminance
 */
float light_calculate_minimal_luminance(light_descriptor ld) {
	return ld.luminance * light_minimal_luminance_multiplier;
}

/*
 *	Calculates the incident ray in eye-space from position
 */
vec3 light_incidant_ray(light_descriptor ld, vec3 position) {
	if (ld.type == LightTypeDirectional) return -ld.transformed_position;
	else return ld.transformed_position - position;
}

/*
 *	Calculate light irradiance at specific distance. 
 *	This method multiplies light irradiance by attenuation coefficient, from that subtracts the light's minimal luminance so that at
 *	distance greater than or equal to the light's effective range the value returned is 0.
 */
vec3 light_lux_at_distance(light_descriptor ld, float dist, float min_lum) {
	if (ld.type == LightTypeDirectional) {
		return ld.diffuse * ld.luminance;
	}
	
	float a = max(.0f, dist / ld.radius);
	float f = 1.f / (1.f + a*a);

	float incident_radiance = ld.luminance * f - min_lum;
	return ld.diffuse * max(0.f, incident_radiance);
}

/*
 *	Calculate light irradiance factoring in distance attenuation and atmospheric attenuation.
 *
 *	@param ld		Light descriptor.
 *	@param dist		Precomputed path distance, from light point to position.
 *	@param position	Transformed, eye space, position.
 *	@param min_lum	Minimal light luminance. Refer to light_lux_at_distance.
 */
vec3 irradiance(light_descriptor ld, float dist, vec3 position, float min_lum) {
	if (ld.type == LightTypeDirectional) {
		//! TODO
		return light_lux_at_distance(ld, .0f, min_lum);
	}
	else {
		// Approximate atmospheric attenuation over the path by using the attenuation coefficients at the mid point
		vec3 lp = ld.transformed_position;
		vec3 mid = mix(lp, position, .5f);
		vec3 att_coef = atmospherics_attenuation_coeffcient(mid);
		vec3 att = beer_lambert(att_coef, dist);

		return light_lux_at_distance(ld, dist, min_lum) * att;
	}
}
/*
 *	Calculate light irradiance factoring in distance attenuation and atmospheric attenuation.
 *
 *	@param ld		Light descriptor.
 *	@param dist		Precomputed path distance, from light point to position.
 *	@param position	Transformed, eye space, position.
 */
vec3 irradiance(light_descriptor ld, float dist, vec3 position) {
	float min_lum = light_calculate_minimal_luminance(ld);
	return irradiance(ld, dist, position, min_lum);
}
/*
 *	Calculate light irradiance factoring in distance attenuation and atmospheric attenuation.
 *
 *	@param ld		Light descriptor.
 *	@param position	Transformed, eye space, position.
 */
vec3 irradiance(light_descriptor ld, vec3 position) {
	if (ld.type == LightTypeDirectional) {
		return irradiance(ld, .0f, position);
	}
	else {
		vec3 i = ld.transformed_position - position;
		float dist = sqrt(dot(i,i));
		return irradiance(ld, dist, position);
	}
}

/*
 *	Calculate light irradiance scattered from position factoring in distance attenuation and atmospheric attenuation.
 *
 *	@param ld			Light descriptor.
 *	@param position		Transformed, eye space, position.
 *	@param volume		Volume of area around position where scattering happens.
 *	@param incident		Sampling direction of light incident ray, originating from position, normalized, in eye space.
 *	@param dist			Precomputed path distance, i.e. length of incident ray.
 *	@param scatter_dir	Sampling direction of out-scatter ray, originating from position, normalized, in eye space.
 *	@param scatter_dist	Out-scatter ray length
 *	@param min_lum		Minimal light luminance. Refer to light_lux_at_distance.
 */
vec3 scatter(light_descriptor ld, 
			 vec3 position, 
			 float volume, 
			 vec3 incident, 
			 float dist, 
			 vec3 scatter_dir, 
			 float scatter_dist,
			 float min_lum) {
	vec3 irradiance = irradiance(ld, dist, position, min_lum);
	float particle_density = atmospherics_air_density(position);
	vec3 extinction_coef = atmospherics_extinction_coeffcient();

	vec3 i = incident;
	vec3 o = scatter_dir;
	float p_mie = cornette_shanks_phase_function(i, o, atmospherics_descriptor_data.phase);
	float p_rayleigh = rayleigh_phase_function(i, o);
	vec3 scatter_coefficient = p_mie * atmospherics_descriptor_data.mie_scattering_coefficient.xxx + 
							   p_rayleigh * atmospherics_descriptor_data.rayleigh_scattering_coefficient;
	scatter_coefficient *= volume * particle_density;

	vec3 att_coef = extinction_coef * particle_density;

	if (ld.type == LightTypeDirectional) {
		//! TODO
		vec3 att = beer_lambert(att_coef, scatter_dist);
		return irradiance * att * scatter_coefficient;
	}
	else {
		vec3 att = beer_lambert(att_coef, dist + scatter_dist);
		return irradiance * att * scatter_coefficient;
	}
}
/*
 *	Calculate light irradiance scattered from position factoring in distance attenuation and atmospheric attenuation.
 *
 *	@param ld			Light descriptor.
 *	@param position		Transformed, eye space, position.
 *	@param volume		Volume of area around position where scattering happens.
 *	@param incident		Sampling direction of light incident ray, originating from position, normalized, in eye space.
 *	@param dist			Precomputed path distance, i.e. length of incident ray.
 *	@param scatter_dir	Sampling direction of out-scatter ray, originating from position, normalized, in eye space.
 *	@param scatter_dist	Out-scatter ray length
 */
vec3 scatter(light_descriptor ld, 
			 vec3 position, 
			 float volume, 
			 vec3 incident, 
			 float dist, 
			 vec3 scatter_dir, 
			 float scatter_dist) {
	float min_lum = light_calculate_minimal_luminance(ld);
	return scatter(ld, 
				   position, 
				   volume, 
				   incident, 
				   dist,
				   scatter_dir, 
				   scatter_dist, 
				   min_lum);
}
