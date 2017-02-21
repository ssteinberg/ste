
#include <common.glsl>
#include <shadow.glsl>
#include <light_transport.glsl>

#include <light.glsl>
#include <girenderer_transform_buffer.glsl>

int subsurface_scattering_calculate_steps(float thickness) {
	const int max_samples = 4;
	const float thickness_multiplier = 1.f / 3.f;

	return max(int(ceil(thickness * thickness_multiplier)), max_samples);
}

/*
 *	Subsurface scattering approximation
 *
 *	@param descriptor	Material layer descriptor
 *	@param position		Eye space position
 *	@param n			Normal
 *	@param thickness	Object thickness at shaded fragment
 *	@param ld			Light descriptor
 *	@param shadow_maps	Shadow maps
 *	@param light		Light index
 *	@param view_ray		Normalized vector from eye to position
 *	@param frag_coords	Screen space coordinates
 */
vec3 subsurface_scattering(material_layer_unpacked_descriptor descriptor,
						   vec3 position,
						   vec3 n,
						   float thickness,
						   light_descriptor ld,
						   light_shading_parameters light,
						   vec3 view_ray,
						   ivec2 frag_coords) {
	const float minimal_attenuation_for_effective_thickness = epsilon;

	vec3 albedo = descriptor.albedo.rgb;
	vec3 attenuation_coefficient = descriptor.attenuation_coefficient;
	float g = descriptor.phase_g;

	float l_radius = ld.radius;
	vec3 l_pos = ld.position;

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

		vec3 shadow_occluder_v = vec3(.0f);/*shadow_occluder(light, 
												 shadow_v, 
												 vec3(0,1,0), vec3(0,1,0),
												 l_radius, 
												 light_effective_range(ld),
												 frag_coords);*/

		float dist_light_to_sample = length(shadow_v);
		float dist_light_to_object = min(length(shadow_occluder_v), dist_light_to_sample);
		float path_length = dist0 + (dist_light_to_sample - dist_light_to_object);
		
		vec3 incident = light_incidant_ray(ld, p) / dist_light_to_sample;
		vec3 irradiance = irradiance(ld);
		vec3 attenuation = exp(-path_length * attenuation_coefficient) * virtual_light_attenuation(ld, dist_light_to_object);
		float phase = henyey_greenstein_phase_function(incident, view_ray, g);

		vec3 scattering = phase * albedo * attenuation;

		samples_accum += scattering * irradiance;
	}

	return samples_accum / float(steps);
}
