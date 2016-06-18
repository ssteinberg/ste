
#include "common.glsl"
#include "interleaved_gradient_noise.glsl"

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
	const vec2 cluster_samples[8] = { vec2(-.7071f,  .7071f),
									  vec2( .0000f,	-.8750f),
									  vec2( .5303f,	 .5303f),
									  vec2(-.6250f,	-.0000f),
									  vec2( .3536f,	-.3536f),
									  vec2(-.0000f,	 .3750f),
									  vec2(-.1768f,	-.1768f),
									  vec2( .1250f,	 .0000f) };

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
	float p = max(3.f, w_penumbra / 2.f);
	int clusters_to_sample = int(ceil(p / 2.f));
	
	float noise = 2.f * pi * interleaved_gradient_noise(gl_FragCoord.xy);

	float accum = .0f;
	float w = .0f;
	for (int i=0; i<clusters_to_sample; ++i) {
		float flip = (i%2) == 0 ? -1.f : 1.f;

		float noise_offset = 2.f * pi * float(i >> 1) / float((clusters_to_sample >> 1) + 1);
		float sin_noise = sin(noise + noise_offset);
		float cos_noise = cos(noise + noise_offset);
		mat2 sample_rotation_matrix = mat2(cos_noise, sin_noise, -sin_noise, cos_noise);

		for (int s=0; s<8; ++s) {
			vec2 u = flip * p * (sample_rotation_matrix * cluster_samples[s]);
			
			const float gaussian = 6.f;
			float l = length(cluster_samples[s]) * p;
			float t = exp(-l*l / (2 * gaussian * gaussian)) / (2 * pi * gaussian * gaussian);

			accum += texture(shadow_depth_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, u), light), zf).x * t;
			w += t;
		}
	}

	return accum /= w;
}

float shadow_fast(samplerCubeArrayShadow shadow_depth_maps, uint light, vec3 shadow_v) {
	vec3 v = abs(shadow_v);
	float m = max(v.x, max(v.y, v.z));

	float zf = shadow_near / m;

	return texture(shadow_depth_maps, vec4(shadow_v, light), zf).x;
}
