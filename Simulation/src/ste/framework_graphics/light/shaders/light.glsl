
#include "common.glsl"

#include "quaternion.glsl"
#include "dual_quaternion.glsl"

#include "girenderer_transform_buffer.glsl"

const int LightTypeSphere = 0;
const int LightTypeDirectional = 1;

const int max_active_lights_per_frame = 32;

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
 *	Light's attenuation coefficient for a given distance to receiver
 */
float light_attenuation_factor(light_descriptor ld, float dist) {
	if (ld.type == LightTypeDirectional)
		return 1;
	else {
		float a = max(.0f, dist / ld.radius);
		float f = 1.f / (1.f + a*a);

		return f;
	}
}

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
	return ld.type == LightTypeSphere ?
				dquat_mul_vec(transform, ld.position) :
				quat_mul_vec(transform.real, ld.position);
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
	return ld.luminance * .000006f;
}

/*
 *	Calculates the incident ray in view-space for position
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
vec3 light_irradiance(light_descriptor ld, float dist) {
	float attenuation_factor = light_attenuation_factor(ld, dist);
	float incident_radiance = ld.luminance * attenuation_factor - light_calculate_minimal_luminance(ld);
	return ld.diffuse * max(0.f, incident_radiance);
}
