
#include "common.glsl"

float shadow_gather_pcf(samplerCubeArrayShadow shadow_depth_maps, uint light, float zf, vec3 norm_v, vec3 v, float m, float r, int samples, float jitter) {
	const float map_size = 1.f / 512.f;

	float pcf = .0f;
	for (int j = 0; j < samples; ++j) {
		float a = (float(j) + jitter) / float(samples);
		vec2 xy = vec2(sin(2.f * pi * a), cos(2.f * pi * a)) * r;

		vec3 u;
		if (v.x == m)		u = vec3(0, xy);
		else if (v.y == m)	u = vec3(xy.x, 0, xy.y);
		else				u = vec3(xy, 0);

		pcf += texture(shadow_depth_maps, vec4(norm_v + u * map_size, light), zf).x;
	}

	return pcf;
}

float shadow_penumbra_width(samplerCubeArrayShadow shadow_depth_maps, uint light, vec3 shadow_v, float l_radius, float dist, float proj22, float proj23) {
	const int samples_far = 3;
	const int samples_med = 3;
	const int samples_near = 3;
	const float radius_far = 6.f;
	const float radius_med = 4.f;
	const float radius_near = 2.f;

	const int total_samples = samples_far + samples_med + samples_near + 1;

	const float weight_far_per_sample = .3f;
	const float weight_med_per_sample = .6f;
	const float weight_near_per_sample = .8f;
	const float weight_center_per_sample = 1.f;
	const float total_weight = weight_center_per_sample +
							   weight_far_per_sample  * samples_far +
							   weight_med_per_sample  * samples_med +
							   weight_near_per_sample * samples_near;

	const float weight_center = 1.f / total_weight;
	const float weight_far = weight_far_per_sample / total_weight;
	const float weight_med = weight_med_per_sample / total_weight;
	const float weight_near = weight_near_per_sample / total_weight;

	vec3 v = abs(shadow_v);
	float m = max(v.x, max(v.y, v.z));

	float ndc_zf = proj22 - proj23 / m;
	float zf = (ndc_zf + 1.f) * .5f;

	vec3 norm_v = shadow_v / m;

	float center = texture(shadow_depth_maps, vec4(shadow_v, light), zf).x * weight_center_per_sample;
	float far = shadow_gather_pcf(shadow_depth_maps, light, zf, norm_v, v, m, radius_far, samples_far, .45);

	float t = (center + far) / float(samples_far + 1);
	if (t >= .999f || t <= .001f)
		return t;

	float pcf = center * weight_center + far * weight_far;
	pcf += shadow_gather_pcf(shadow_depth_maps, light, zf, norm_v, v, m, radius_med, samples_med, .7) * weight_med;
	pcf += shadow_gather_pcf(shadow_depth_maps, light, zf, norm_v, v, m, radius_near, samples_near, .2) * weight_near;

	return pcf;
}
