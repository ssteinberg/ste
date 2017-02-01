
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
	uint type;
	
	// directional_distance: For directional lights only, distance from origin opposite to lights direction (i.e. -directional_distance*position)
	float directional_distance;
	uint cascade_idx;
	
	//! The rest is used internally only

	// shadow_face_mask: For masking (culling) shadow map cube faces (for spherical) or cascades (for directional).
	uint shadow_face_mask;
	
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
	return ld.radius * (sqrt(ld.luminance / l - 1.f) + 1.f);
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
 *	Calculate light irradiance illuminating from light source at 0 distance. 
 *
 *	@param ld			Light descriptor.
 */
vec3 irradiance(light_descriptor ld) {
	float min_lum = ld.type == LightTypeDirectional ? 
		.0f : 
		light_calculate_minimal_luminance(ld);
	return ld.diffuse * ld.luminance - min_lum;
}

/*
 *	Calculate light irradiance at specific distance. 
 *	This method multiplies light irradiance by attenuation coefficient, from that subtracts the light's minimal luminance so that at
 *	distance greater than or equal to the light's effective range the value returned is 0.
 *
 *	@param ld			Light descriptor.
 *	@param dist			Precomputed path distance, from light point to position.
 *	@param min_lum		Minimal light luminance. Refer to light_lux_at_distance.
 */
vec3 irradiance(light_descriptor ld, float dist, float min_lum) {
	if (ld.type == LightTypeDirectional) {
		return irradiance(ld);
	}
	
	float a = max(.0f, dist / ld.radius - 1.f);
	float f = 1.f / (1.f + a*a);

	float illuminance = max(0.f, ld.luminance * f - min_lum);
	return ld.diffuse * illuminance;
}
/*
 *	See irradiance(ld, dist, min_lum) for more details.
 *
 *	@param ld			Light descriptor.
 *	@param dist			Precomputed path distance, from light point to position.
 */
vec3 irradiance(light_descriptor ld, float dist) {
	float min_lum = light_calculate_minimal_luminance(ld);
	return irradiance(ld, dist, min_lum);
}
