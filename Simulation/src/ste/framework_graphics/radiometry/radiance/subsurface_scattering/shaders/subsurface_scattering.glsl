
#include "common.glsl"
#include "shadow.glsl"

#include "light_load.glsl"
#include "girenderer_transform_buffer.glsl"

int subsurface_scattering_calculate_steps(float thickness) {
	return 3;//ceil(thickness / 10.f);
}

vec3 subsurface_scattering(material_layer_unpacked_descriptor descriptor,
						   vec3 base_color,
						   vec3 position,
						   vec3 n,
						   float outer_back_layers_attenuation,
						   float thickness,
						   light_descriptor ld,
						   samplerCubeArray shadow_maps, uint light,
						   vec3 view_ray) {
	float attenuation_coefficient = descriptor.attenuation_coefficient;
	float l_radius = ld.radius;
	vec3 l_pos = ld.position;
	
	if (attenuation_coefficient <= .0f || outer_back_layers_attenuation < .01f)
		return vec3(.0f);

	int steps = subsurface_scattering_calculate_steps(thickness);
	vec3 samples_accum = vec3(0);

	for (int i = 0; i < steps; ++i) {
		float dist0 = (float(i) + .5f) / float(steps) * thickness;

		vec3 p = position + dist0 * view_ray;
		vec3 w_pos = dquat_mul_vec(view_transform_buffer.inverse_view_transform, p);
		vec3 shadow_v = w_pos - l_pos;

		float dist_light_to_sample = length(shadow_v);
		float dist_light_to_object = min(shadow_dist(shadow_maps, light, shadow_v, l_radius), dist_light_to_sample);
		float path_length = dist0 + (dist_light_to_sample - dist_light_to_object);

		vec3 irradiance = light_irradiance(ld, dist_light_to_object) * outer_back_layers_attenuation;

		float attenuation = exp(-path_length * attenuation_coefficient);
		vec3 scattering = attenuation * irradiance;

		samples_accum += base_color * scattering;
	}

	return samples_accum / float(steps);
}
