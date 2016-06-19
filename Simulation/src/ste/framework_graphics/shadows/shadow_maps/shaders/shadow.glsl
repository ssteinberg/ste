
#include "common.glsl"
#include "interleaved_gradient_noise.glsl"

const float shadow_near = 20.f;
const float shadow_map_size = 1024.f;

vec3 shadow_cubemap_jitter_uv(vec3 norm_v, mat2x3 m, vec2 xy) {
	vec3 u = m * xy;
	return norm_v + u / shadow_map_size;
}

float shadow_blocker_search(samplerCubeArray shadow_maps, uint light, vec3 norm_v, mat2x3 m) {
	float d =  texture(shadow_maps, vec4(norm_v, light)).x;
	d = min(d, texture(shadow_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2(-1.5f, -1.5f)), light)).x);
	d = min(d, texture(shadow_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2(-1.5f,  1.5f)), light)).x);
	d = min(d, texture(shadow_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2( 1.5f, -1.5f)), light)).x);
	d = min(d, texture(shadow_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, vec2( 1.5f,  1.5f)), light)).x);

	return d;
}

float shadow(samplerCubeArrayShadow shadow_depth_maps, samplerCubeArray shadow_maps, uint light, vec3 shadow_v, float dot_v_n, float light_radius) {
	const vec2 cluster_samples[8] = { vec2(-.7071f,  .7071f),
									  vec2( .0000f,	-.8750f),
									  vec2( .5303f,	 .5303f),
									  vec2(-.6250f,	-.0000f),
									  vec2( .3536f,	-.3536f),
									  vec2(-.0000f,	 .3750f),
									  vec2(-.1768f,	-.1768f),
									  vec2( .1250f,	 .0000f) };
	const float penumbra_scale = 5.f;
	const float min_v_dot_n = .075f;
	const int max_cluster_samples = 12;
	const float penumbra_w_to_clusters_ratio = .05f;
	const float gaussian = 10.f;

	float dist_receiver;
	mat2x3 m;
	{
		vec3 v = abs(shadow_v);
		dist_receiver = max(v.x, max(v.y, v.z));

		if (v.x == dist_receiver)		m = mat2x3(vec3(0,1,0), vec3(0,0,1));
		else if (v.y == dist_receiver)	m = mat2x3(vec3(1,0,0), vec3(0,0,1));
		else							m = mat2x3(vec3(1,0,0), vec3(0,1,0));
	}

	float zf = shadow_near / dist_receiver;
	vec3 norm_v = shadow_v / dist_receiver;

	float d_blocker = shadow_near / shadow_blocker_search(shadow_maps, light, norm_v, m);
	float w_penumbra = max(.0f, dist_receiver - d_blocker) * light_radius / d_blocker;
	
	float scale = 1.f / max(min_v_dot_n, abs(dot_v_n));
	float map_w_penumbra = penumbra_scale * max(w_penumbra / dist_receiver, 1.f) * scale;

	int clusters_to_sample = min(max_cluster_samples, int(ceil(map_w_penumbra * penumbra_w_to_clusters_ratio)));
	float noise = 2.f * pi * interleaved_gradient_noise(gl_FragCoord.xy);

	float accum = .0f;
	float w = .0f;
	for (int i=0; i<clusters_to_sample; ++i) {
		float flip = (i%2) == 0 ? -1.f : 1.f;

		float noise_offset = 2.f * pi * float((i >> 1) << 1) / float(clusters_to_sample | 1);
		float sin_noise = sin(noise + noise_offset);
		float cos_noise = cos(noise + noise_offset);
		mat2 sample_rotation_matrix = mat2(cos_noise, sin_noise, -sin_noise, cos_noise);

		for (int s=0; s<8; ++s) {
			vec2 u = flip * map_w_penumbra * (sample_rotation_matrix * cluster_samples[s]);
			
			float l = length(cluster_samples[s]);
			float t = exp(-l*l / (2 * gaussian * gaussian)) / (2 * pi * gaussian * gaussian);

			accum += texture(shadow_depth_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, u), light), zf).x * t;
			w += t;
		}

		if (i == 0 && (accum <= .001f || accum >= w * .999f))
			return accum / w;
	}

	return accum / w;
}

float shadow_fast(samplerCubeArrayShadow shadow_depth_maps, uint light, vec3 shadow_v) {
	vec3 v = abs(shadow_v);
	float m = max(v.x, max(v.y, v.z));

	float zf = shadow_near / m;

	return texture(shadow_depth_maps, vec4(shadow_v, light), zf).x;
}
