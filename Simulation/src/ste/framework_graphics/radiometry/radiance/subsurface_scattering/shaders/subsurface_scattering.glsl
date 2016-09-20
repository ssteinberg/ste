
#include "common.glsl"
#include "shadow.glsl"
#include "light_transport.glsl"

#include "light_load.glsl"
#include "girenderer_transform_buffer.glsl"

int subsurface_scattering_calculate_steps(float thickness) {
	const int max_samples = 4;
	const float thickness_multiplier = 1.f / 3.f;

	return min(max_samples, int(ceil(thickness * thickness_multiplier)));
}

vec3 subsurface_scattering(material_layer_unpacked_descriptor descriptor,
						   vec3 base_color,
						   vec3 position,
						   vec3 n,
						   vec3 outer_back_layers_attenuation,
						   float thickness,
						   light_descriptor ld,
						   samplerCubeArray shadow_maps, uint light,
						   vec3 view_ray) {
	const float minimal_attenuation_for_effective_thickness = .000001f;

	vec3 attenuation_coefficient = descriptor.attenuation_coefficient;
	float g = descriptor.phase_g;

	float l_radius = ld.radius;
	vec3 l_pos = ld.position;
	
	if (all(lessThanEqual(attenuation_coefficient, vec3(.0f))) || 
		all(lessThanEqual(outer_back_layers_attenuation, vec3(.0001f))))
		return vec3(.0f);

	vec3 depth_to_reach_minimal_attenuation3 = vec3(-log(minimal_attenuation_for_effective_thickness)) / attenuation_coefficient;
	float depth_to_reach_minimal_attenuation = max(depth_to_reach_minimal_attenuation3.x, max(depth_to_reach_minimal_attenuation3.y, depth_to_reach_minimal_attenuation3.z));
	float effective_thickness = min(thickness, depth_to_reach_minimal_attenuation);
	int steps = subsurface_scattering_calculate_steps(effective_thickness);

	vec3 samples_accum = vec3(0);
	for (int i = 0; i < steps; ++i) {
		float dist0 = (float(i) + .5f) / float(steps) * effective_thickness;

		vec3 p = position + dist0 * view_ray;
		vec3 w_pos = dquat_mul_vec(view_transform_buffer.inverse_view_transform, p);
		vec3 shadow_v = w_pos - l_pos;

		vec3 shadow_occluder_v = shadow_occluder(shadow_maps, light, shadow_v, l_radius);

		float dist_light_to_sample = length(shadow_v);
		float dist_light_to_object = min(length(shadow_occluder_v), dist_light_to_sample);
		float path_length = dist0 + (dist_light_to_sample - dist_light_to_object);
		
		vec3 incident = light_incidant_ray(ld, p) / dist_light_to_sample;
		vec3 irradiance = light_irradiance(ld, dist_light_to_object) * outer_back_layers_attenuation;
		vec3 attenuation = exp(-path_length * attenuation_coefficient);
		float phase = henyey_greenstein_phase_function(incident, view_ray, g);

		vec3 scattering = phase * attenuation * irradiance;

		samples_accum += base_color * scattering;
	}

	return samples_accum / float(steps);
}
