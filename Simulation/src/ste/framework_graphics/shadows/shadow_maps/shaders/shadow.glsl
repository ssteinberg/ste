
#include "common.glsl"
#include "fast_rand.glsl"

const float shadow_near = 20.f;

float shadow_gather_pcf(samplerCubeArrayShadow shadow_depth_maps, uint light, float zf, vec3 norm_v, vec3 v, float m, vec2 xy) {
	vec3 u;
	if (v.x == m)		u = vec3(0, xy);
	else if (v.y == m)	u = vec3(xy.x, 0, xy.y);
	else				u = vec3(xy, 0);

	return texture(shadow_depth_maps, vec4(norm_v + u, light), zf).x;
}

float shadow(samplerCubeArrayShadow shadow_depth_maps, uint light, vec3 shadow_v) {
	const int rings = 3;
	const int samples = 4;
	const float map_size = 1.f / 512.f;

	vec3 v = abs(shadow_v);
	float m = max(v.x, max(v.y, v.z));

	float zf = shadow_near / m;
	vec3 norm_v = shadow_v / m;

	float accum = texture(shadow_depth_maps, vec4(shadow_v, light), zf).x;

	vec2 wh = vec2(map_size);

	float s = 1.f;
	for (int i = rings; i >= 1; --i) {
		const int ringsamples = samples;
		float jitter = fast_rand(float(i) * shadow_v.xy);

		for (int j = 0; j < ringsamples; ++j) {
			float step = (float(j) + jitter) * pi*2.f / float(ringsamples);
			vec2 c = vec2(cos(step), sin(step)) * float(i) * 1.5f;
			float w = mix(1.f, float(rings - i + 1) / float(rings + 1), .75f) * .9f;

			accum += shadow_gather_pcf(shadow_depth_maps, light, zf, norm_v, v, m, c * wh) * w;
			s += w;
		}

		if (rings == i) {
			if (accum / s == .99f || accum / s == .01f)
				return accum / s;
		}
	}

	return accum / s;
}

float shadow_fast(samplerCubeArrayShadow shadow_depth_maps, uint light, vec3 shadow_v) {
	vec3 v = abs(shadow_v);
	float m = max(v.x, max(v.y, v.z));

	float zf = shadow_near / m;

	return texture(shadow_depth_maps, vec4(shadow_v, light), zf).x;
}
