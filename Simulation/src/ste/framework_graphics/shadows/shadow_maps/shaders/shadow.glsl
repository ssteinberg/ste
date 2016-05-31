
#include "common.glsl"

const float shadow_near = 20.f;

vec3 shadow_cubemap_jitter_uv(vec3 norm_v, mat2x3 m, vec2 xy) {
	const float shadow_map_size = 512.f;

	vec3 u = m * xy;
	return norm_v + u / shadow_map_size;
}

float shadow_blocker_search(samplerCubeArray shadow_maps, uint light, vec3 norm_v, mat2x3 m) {
	float d =  texture(shadow_maps, vec4(norm_v, light)).x;
	d = max(d, texture(shadow_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2(-2, -2)), light)).x);
	// d = max(d, texture(shadow_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2(-2,  0)), light)).x);
	d = max(d, texture(shadow_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2(-2,  2)), light)).x);
	// d = max(d, texture(shadow_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2( 0, -2)), light)).x);
	// d = max(d, texture(shadow_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2( 0,  2)), light)).x);
	d = max(d, texture(shadow_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2( 2, -2)), light)).x);
	// d = max(d, texture(shadow_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2( 2,  0)), light)).x);
	d = max(d, texture(shadow_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2( 2,  2)), light)).x);

	return d;
}

float shadow(samplerCubeArrayShadow shadow_depth_maps, samplerCubeArray shadow_maps, uint light, vec3 shadow_v, float light_radius) {
	const float pcf_step = 2.f;
	const float weight_bias = .6f;

	vec3 v = abs(shadow_v);
	float dist_receiver = max(v.x, max(v.y, v.z));

	mat2x3 m;
	if (v.x == dist_receiver)		m = mat2x3(vec3(0,1,0), vec3(0,0,1));
	else if (v.y == dist_receiver)	m = mat2x3(vec3(1,0,0), vec3(0,0,1));
	else							m = mat2x3(vec3(1,0,0), vec3(0,1,0));

	float zf = shadow_near / dist_receiver;
	vec3 norm_v = shadow_v / dist_receiver;

	float d_blocker = shadow_near / shadow_blocker_search(shadow_maps, light, norm_v, m);

	if (dist_receiver - d_blocker <= .0f)
		return 1.f;

	float w_penumbra = (dist_receiver - d_blocker) * light_radius / d_blocker;
	float p = w_penumbra / pcf_step / dist_receiver;
	int samples = clamp(int(round(p)), 1, 3);

	float accum = texture(shadow_depth_maps, vec4(shadow_v, light), zf).x;
	float tw = 1.f;

	if (samples > 0) {
		float t = .0f;
		t += texture(shadow_depth_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2(-samples,-samples) * pcf_step), light), zf).x;
		t += texture(shadow_depth_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2(-samples, samples) * pcf_step), light), zf).x;
		t += texture(shadow_depth_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2( samples,-samples) * pcf_step), light), zf).x;
		t += texture(shadow_depth_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2( samples, samples) * pcf_step), light), zf).x;

		float far = (accum + t) * .2f;
		if (far <= .01f && far >= .99f)
			return far;

		float wt = 1.f - float(samples) / float(samples + 1);
		float w = mix(1.f, wt * wt, weight_bias);

		accum += t * w;
		tw += 4.f * w;
	}

	for (int x=-samples; x<=samples; ++x) {
		for (int y=-samples; y<=samples; ++y) {
			if (abs(x) == samples && abs(y) == samples)
				continue;
			if (x == 0 && y == 0)
				continue;

			float wx = 1.f - float(x) / float(samples + 1);
			float wy = 1.f - float(y) / float(samples + 1);
			float w = mix(1.f, wx * wy, weight_bias);

			accum += texture(shadow_depth_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2(x,y) * pcf_step), light), zf).x * w;
			tw += w;
		}
	}

	return accum /= tw;
}

float shadow_fast(samplerCubeArrayShadow shadow_depth_maps, uint light, vec3 shadow_v) {
	vec3 v = abs(shadow_v);
	float m = max(v.x, max(v.y, v.z));

	float zf = shadow_near / m;

	return texture(shadow_depth_maps, vec4(shadow_v, light), zf).x;
}
