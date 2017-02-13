
#include "light_type.glsl"

#include "common.glsl"
#include "pack.glsl"
#include "chromaticity.glsl"

#include "quaternion.glsl"
#include "dual_quaternion.glsl"

#include "girenderer_transform_buffer.glsl"

#include "light_transport.glsl"
#include "atmospherics.glsl"

const int max_active_lights_per_frame = 24;
const int max_active_directional_lights_per_frame = 4;
const int total_max_active_lights_per_frame = max_active_lights_per_frame + max_active_directional_lights_per_frame;

struct light_descriptor {
	// position: Light position for spherical, direction for directional lights.
	// Radius is light's radius (all light types)
	// Emittance is the emitted luminance (all light types)
	vec3 position;	float radius;
	vec3 emittance;	uint type;
	// Texture
	layout(bindless_sampler) sampler2D texture;
	// Light effective range.
	// For directional lights: distance from origin opposite to lights direction (i.e. -directional_distance*position)
	float effective_range_or_directional_distance;
	// polygonal_light_points_and_offset specifies the number of points and offset into the buffer (polygonal lights only)
	uint polygonal_light_points_and_offset_or_cascade_idx;
	
	//! The rest is used internally only
	
	// transformed_position: Light position in eye space for spherical, direction in eye space for directional lights.
	vec3 transformed_position;

	float _unused;
};

/*
 *	Transforms light's position/direction based on transformation dual quaternion
 */
vec3 light_transform(dual_quaternion transform, light_descriptor ld) {
	return light_type_is_directional(ld.type) ?
				quat_mul_vec(transform.real, ld.position) :
				dquat_mul_vec(transform, ld.position);
}

/*
 *	Returns the light effective range
 */
float light_effective_range(light_descriptor ld) {
	if (light_type_is_directional(ld.type)) return +inf;
	else return ld.effective_range_or_directional_distance;
}

/*
 *	Returns the light directional distance, applicable for directional lights only
 */
float light_directional_distance(light_descriptor ld) {
	return ld.effective_range_or_directional_distance;
}

/*
 *	Calculates the incident ray in eye-space for sampling position
 */
vec3 light_incidant_ray(light_descriptor ld, vec3 position) {
	if (light_type_is_directional(ld.type)) return -ld.transformed_position;
	else return ld.transformed_position - position;
}

/*
 *	Calculate light attenuation at specified distance. Applicable for virtual lights, approximates other lights.
 *
 *	@param ld			Light descriptor.
 *	@param dist			Distance.
 */
float virtual_light_attenuation(light_descriptor ld, float dist) {
	float a = max(.0f, dist / ld.radius - 1.f);
	return 1.f / (1.f + a*a);
}

/*
 *	Returns the light irradiance illuminating from light source at 0 distance. 
 *
 *	@param ld			Light descriptor.
 */
vec3 irradiance(light_descriptor ld) {
	return ld.emittance;
}

/*
 *	Get polygonal light points count
 *
 *	@param ld			Light descriptor.
 */
uint light_get_polygon_point_counts(light_descriptor ld) {
	return ld.polygonal_light_points_and_offset_or_cascade_idx >> 24;
}

/*
 *	Get polygonal light offset into points buffer
 *
 *	@param ld			Light descriptor.
 */
uint light_get_polygon_point_offset(light_descriptor ld) {
	return ld.polygonal_light_points_and_offset_or_cascade_idx & 0x00FFFFFF;
}
