
#include "common.glsl"
#include "interleaved_gradient_noise.glsl"

#include "project.glsl"

const float shadow_map_size = 1024.f;

float shadow_calculate_test_depth(float zf) {
	return zf * 1.015f + 8.f * epsilon;
}

vec3 shadow_cubemap_jitter_uv(vec3 norm_v, mat2x3 m, vec2 xy) {
	vec3 u = m * xy;
	return norm_v + u;
}

float shadow_blocker_search(samplerCubeArray shadow_maps, uint light, vec3 norm_v, mat2x3 m) {
	float d = texture(shadow_maps, vec4(norm_v, light)).x;
	return d;
}

float shadow_calculate_penumbra(float d_blocker, float radius, float dist_receiver) {
	const float penumbra_scale = 1.f;

	float w_penumbra = (dist_receiver - d_blocker) * radius / d_blocker;
	return penumbra_scale * w_penumbra / dist_receiver;
}

float shadow(samplerCubeArrayShadow shadow_depth_maps, samplerCubeArray shadow_maps, uint light, vec3 shadow_v, float light_radius) {
	const vec2 cluster_samples[8] = { vec2(-.7071f,  .7071f),
									  vec2( .0000f,	-.8750f),
									  vec2( .5303f,	 .5303f),
									  vec2(-.6250f,	-.0000f),
									  vec2( .3536f,	-.3536f),
									  vec2(-.0000f,	 .3750f),
									  vec2(-.1768f,	-.1768f),
									  vec2( .1250f,	 .0000f) };

	const float max_cluster_samples = 10.f;
	const float max_penumbra = 80.f / shadow_map_size;
	const float min_penumbra = 6.f / shadow_map_size;
	const float penumbra_w_to_clusters_ratio_power = 1.f;

	const float cutoff = .5f;

	float shadow_near = light_radius;

	float dist_receiver;
	mat2x3 m;
	{
		vec3 v = abs(shadow_v);
		dist_receiver = max_element(v);

		if (v.x == dist_receiver)		m = mat2x3(vec3(0,1,0), vec3(0,0,1));
		else if (v.y == dist_receiver)	m = mat2x3(vec3(1,0,0), vec3(0,0,1));
		else							m = mat2x3(vec3(1,0,0), vec3(0,1,0));
	}

	float zf = shadow_near / dist_receiver;
	vec3 norm_v = shadow_v / dist_receiver;
	float d_blocker = shadow_near / shadow_blocker_search(shadow_maps, light, norm_v, m);

	if (dist_receiver - d_blocker <= .0f)
		return 1.f;

	float penumbra = clamp(shadow_calculate_penumbra(d_blocker, light_radius, dist_receiver) / cutoff, min_penumbra, max_penumbra);

	float penumbra_ratio = pow((penumbra - min_penumbra) / (max_penumbra - min_penumbra), penumbra_w_to_clusters_ratio_power);
	int clusters_to_sample = int(round(mix(.51f, max_cluster_samples + .49f, penumbra_ratio)));

	float noise = 2.f * pi * interleaved_gradient_noise(gl_FragCoord.xy);
	float accum = .0f;
	float w = .0f;

	for (int i=0; i<clusters_to_sample; ++i) {
		float noise_offset = 2.f * pi * float(i) / float(clusters_to_sample);
		float sin_noise = sin(noise + noise_offset);
		float cos_noise = cos(noise + noise_offset);
		mat2 sample_rotation_matrix = mat2(cos_noise, sin_noise, -sin_noise, cos_noise);

		for (int s=0; s<8; ++s) {
			vec2 u = penumbra * (sample_rotation_matrix * cluster_samples[s]);
			
			float gaussian = penumbra;
			float l = length(cluster_samples[s]) * penumbra;
			float t = exp(-l*l / (2 * gaussian * gaussian)) / (2 * pi * gaussian * gaussian);

			float shadow_sample = texture(shadow_depth_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, u), light), shadow_calculate_test_depth(zf)).x;
			accum += shadow_sample * t;
			w += t;
		}

		if (i == 1 && 
			accum <= epsilon)
			break;
	}

	return smoothstep(.0, 1.f, min(1.f, accum / w / cutoff));
}

float shadow_fast(samplerCubeArrayShadow shadow_depth_maps, uint light, vec3 shadow_v, float light_radius) {
	vec3 v = abs(shadow_v);
	float m = max(v.x, max(v.y, v.z));

	float shadow_near = light_radius;

	float zf = shadow_near / m;

	return texture(shadow_depth_maps, vec4(shadow_v, light), shadow_calculate_test_depth(zf)).x;
}

float shadow_depth(samplerCubeArray shadow_maps, uint light, vec3 shadow_v) {
	vec3 v = abs(shadow_v);
	float m = max(v.x, max(v.y, v.z));

	return texture(shadow_maps, vec4(shadow_v, light)).x;
}

float shadow_dist(samplerCubeArray shadow_maps, uint light, vec3 shadow_v, float light_radius) {
	float d = shadow_depth(shadow_maps, light, shadow_v);
	float shadow_near = light_radius;

	return unproject_depth(d, shadow_near);
}
