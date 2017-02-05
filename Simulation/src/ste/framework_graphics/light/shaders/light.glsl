
#include "light_type.glsl"

#include "common.glsl"
#include "chromaticity.glsl"

#include "quaternion.glsl"
#include "dual_quaternion.glsl"

#include "girenderer_transform_buffer.glsl"

#include "light_transport.glsl"
#include "atmospherics.glsl"

const int max_active_lights_per_frame = 24;
const int max_active_directional_lights_per_frame = 4;
const int total_max_active_lights_per_frame = max_active_lights_per_frame + max_active_directional_lights_per_frame;

const float light_minimal_luminance_multiplier = 1e-6f;

struct light_descriptor {
	// position: Light position for spherical, direction for directional lights.
	// Radius is light's radius (all light types)
	// Emittance is the emitted luminance (all light types)
	// polygonal_light_points_and_offset specifies the number of points and offset into the buffer (polygonal lights only)
	vec3 position;	float radius;
	vec3 emittance;	uint polygonal_light_points_and_offset;

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
	return light_type_is_directional(ld.type) ?
				quat_mul_vec(transform.real, ld.position) :
				dquat_mul_vec(transform, ld.position);
}

/*
 *	Calculates light's effective range given desired minimal luminance
 */
float light_calculate_effective_range(light_descriptor ld, float min_lum) {
	float lum = luminance(ld.emittance);
	return ld.radius * (sqrt(lum / min_lum - 1.f) + 1.f);
}

/*
 *	Calculates a suggested light's minimal luminance
 */
float light_calculate_minimal_luminance(light_descriptor ld) {
	return light_type_is_directional(ld.type) ? 
				.0f : 
				luminance(ld.emittance) * light_minimal_luminance_multiplier;
}

/*
 *	Calculates the incident ray in eye-space for sampling position
 */
vec3 light_incidant_ray(light_descriptor ld, vec3 position) {
	if (light_type_is_directional(ld.type)) return -ld.transformed_position;
	else return ld.transformed_position - position;
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
 *	Calculate light attenuation at specified distance. 
 *
 *	@param ld			Light descriptor.
 *	@param dist			Distance.
 */
float light_attenuation(light_descriptor ld, float dist) {
	if (light_type_is_directional(ld.type))
		return 1.f;
	
	float a = max(.0f, dist / ld.radius - 1.f);
	return 1.f / (1.f + a*a);
}

/*
 *	Calculate light irradiance at specific distance. 
 *	This method multiplies light irradiance by attenuation coefficient, from that subtracts the light's minimal luminance so that at
 *	distance greater than or equal to the light's effective range the value returned is 0.
 *
 *	@param ld			Light descriptor.
 *	@param dist			Precomputed path distance, from light point to position.
 *	@param min_lum		Minimal light luminance. 
 */
vec3 irradiance(light_descriptor ld, float dist, float min_lum) {
	float f = light_attenuation(ld, dist);
	return max(vec3(.0f), ld.emittance * f - vec3(min_lum));
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

/*
 *	Get polygonal light points count
 *
 *	@param ld			Light descriptor.
 */
uint light_get_polygon_point_counts(light_descriptor ld) {
	return ld.polygonal_light_points_and_offset >> 24;
}

/*
 *	Get polygonal light offset into points buffer
 *
 *	@param ld			Light descriptor.
 */
uint light_get_polygon_point_offset(light_descriptor ld) {
	return ld.polygonal_light_points_and_offset & 0x00FFFFFF;
}
