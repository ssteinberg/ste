
#include "common.glsl"

const float shadow_delta = 1.f / 1000000.f;

float shadow_gather_pcf(samplerCubeArrayShadow shadow_depth_maps, int light, float zf, vec3 norm_v, vec3 v, float m, float r, int samples, float jitter, out int i) {
	const float map_size = 1.f / 1024.f;

	float pcf = .0f;
	for (int j = 0; j < samples; ++i, ++j) {
		float a = (float(j) + jitter) / float(samples);
		vec2 xy = vec2(sin(2.f * pi * a), cos(2.f * pi * a)) * r;

		vec3 u;
		if (v.x == m)		u = vec3(0, xy);
		else if (v.y == m)	u = vec3(xy.x, 0, xy.y);
		else				u = vec3(xy, 0);

		pcf += texture(shadow_depth_maps, vec4(norm_v + u * map_size, light), zf + shadow_delta).x;
	}

	return pcf;
}

float shadow_penumbra_width(samplerCubeArrayShadow shadow_depth_maps, int light, vec3 shadow_v, float l_radius, float dist, float proj22, float proj23) {
	vec3 v = abs(shadow_v);
	float m = max(v.x, max(v.y, v.z));

	float c;
	if (v.x == m)
		c = v.x;
	else if (v.y == m)
		c = v.y;
	else
		c = v.z;

	float ndc_zf = proj22 - proj23 / c;
	float zf = (ndc_zf + 1.f) * .5f;

	vec3 norm_v = shadow_v / c;

	float pcf = texture(shadow_depth_maps, vec4(norm_v, light), zf + shadow_delta).x;

	const int samples_far = 2;
	const int samples_med = 3;
	const int samples_near = 3;
	const float radius_far = 6.f;
	const float radius_med = 4.f;
	const float radius_near = 2.f;
	int i = 0;

	pcf += shadow_gather_pcf(shadow_depth_maps, light, zf, norm_v, v, m, radius_far, samples_far, .6, i);

	float t = clamp(pcf / float(i + 1), .0f, 1.f);
	if (t >= .99f || t <= .01f)
		return t;

	pcf += shadow_gather_pcf(shadow_depth_maps, light, zf, norm_v, v, m, radius_med, samples_med, .2, i);
	pcf += shadow_gather_pcf(shadow_depth_maps, light, zf, norm_v, v, m, radius_near, samples_near, .7, i);

	return clamp(pcf / float(i + 1), .0f, 1.f);
}
