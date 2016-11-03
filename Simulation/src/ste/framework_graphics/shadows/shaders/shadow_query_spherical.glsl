
#include "shadow_common.glsl"

vec3 shadow_cubemap_jitter_uv(vec3 norm_v, mat2x3 m, vec2 xy) {
	vec3 u = m * xy;
	return norm_v + u;
}

float shadow_blocker_search(samplerCubeArray shadow_maps, uint idx, vec3 norm_v, mat2x3 m) {
	float d = texture(shadow_maps, vec4(norm_v, idx)).x;
	return d;
}

float shadow_calculate_distance_to_receiver(vec3 shadow_v, out mat2x3 m) {
	vec3 v = abs(shadow_v);
	float dist_receiver = max_element(v);

	if (v.x == dist_receiver)		m = mat2x3(vec3(0,1,0), vec3(0,0,1));
	else if (v.y == dist_receiver)	m = mat2x3(vec3(1,0,0), vec3(0,0,1));
	else							m = mat2x3(vec3(1,0,0), vec3(0,1,0));

	return dist_receiver;
}

/*
 *	Calculate percentage of unshadowed light irradiating in direction and distance given by 'shadow_v' from light.
 *	Filters shadow maps using samples from an interleaved gradient noise pattern, based on penumbra size.
 */
float shadow(samplerCubeArrayShadow shadow_depth_maps, samplerCubeArray shadow_maps, uint idx, vec3 shadow_v, float light_radius) {
	const float max_cluster_samples = 10.f;
	const float max_penumbra = 80.f / shadow_cubemap_size;
	const float min_penumbra = 6.f / shadow_cubemap_size;
	const float penumbra_w_to_clusters_ratio_power = 1.3f;
	float shadow_near = light_radius;

	const float cutoff = .5f;

	mat2x3 m;
	float dist_receiver = shadow_calculate_distance_to_receiver(shadow_v, m);

	vec3 norm_v = shadow_v / dist_receiver;
	float d_blocker = project_depth(-shadow_blocker_search(shadow_maps, idx, norm_v, m), shadow_near);

	// No shadowing if distance to blocker is further away from receiver
	if (dist_receiver <= d_blocker)
		return 1.f;
		
	float d = project_depth(-dist_receiver, shadow_near);//shadow_near / dist_receiver;
	float zf = shadow_calculate_test_depth(d);

	// Calculate penumbra based on distance and light radius
	float penumbra = clamp(shadow_calculate_penumbra(d_blocker, light_radius, dist_receiver) / cutoff, min_penumbra, max_penumbra);

	// Number of clusters to sample calculated heuristically using the penumbra size
	float penumbra_ratio = pow((penumbra - min_penumbra) / (max_penumbra - min_penumbra), penumbra_w_to_clusters_ratio_power);
	int clusters_to_sample = int(round(mix(1.51f, max_cluster_samples + .49f, penumbra_ratio)));

	// Interleaved gradient noise
	float noise = two_pi * interleaved_gradient_noise(gl_FragCoord.xy);
	float accum = .0f;
	float w = .0f;

	// Accumulate samples
	// Each cluster is rotated around and offseted by the generated interleaved noise
	for (int i=0; i<clusters_to_sample; ++i) {
		float noise_offset = two_pi * float(i) / float(clusters_to_sample);
		float sin_noise = sin(noise + noise_offset);
		float cos_noise = cos(noise + noise_offset);
		mat2 sample_rotation_matrix = mat2(cos_noise,  sin_noise, 
										   -sin_noise, cos_noise);

		// Constant sample pattern per cluster
		// Calculate the offset, lookup shadowmap texture
		// Weights per sample are Gaussian
		for (int s=0; s<8; ++s) {
			vec2 u = penumbra * (sample_rotation_matrix * shadow_cluster_samples[s]);
			
			float gaussian = penumbra;
			float l = length(shadow_cluster_samples[s]) * penumbra;
			float t = exp(-l*l / (2 * gaussian * gaussian)) * one_over_pi / (2 * gaussian * gaussian);

			float shadow_sample = texture(shadow_depth_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, u), idx), zf).x;
			accum += shadow_sample * t;
			w += t;
		}

		// Stop early if first cluster is fully unshadowed
		if (i == 1 && 
			accum <= epsilon)
			break;
	}

	return smoothstep(.0, 1.f, min(1.f, accum / w / cutoff));
}

/*
 *	Fast unfiltered shadow lookup
 */
float shadow_fast(samplerCubeArrayShadow shadow_depth_maps, uint idx, vec3 shadow_v, float light_radius) {
	vec3 v = abs(shadow_v);
	float m = max(v.x, max(v.y, v.z));

	float shadow_near = light_radius;

	float zf = shadow_near / m;

	return texture(shadow_depth_maps, vec4(shadow_v, idx), shadow_calculate_test_depth(zf)).x;
}

/*
 *	Lookup unfiltered depth from shadowmap
 */
float shadow_depth(samplerCubeArray shadow_maps, uint idx, vec3 shadow_v) {
	vec3 v = abs(shadow_v);
	return texture(shadow_maps, vec4(shadow_v, idx)).x;
}

/*
 *	Lookup shadow occluder in shadow_v direction
 *	Filtered, like 'shadow' method but limited to a single cluster and smaller upper limit on penumbra size
 */
vec3 shadow_occluder(samplerCubeArray shadow_maps, uint idx, vec3 shadow_v, float light_radius) {
	const float cutoff = .5f;
	const float max_penumbra = 20.f / shadow_cubemap_size;
	const float min_penumbra = 4.f / shadow_cubemap_size;
	float shadow_near = light_radius;

	mat2x3 m;
	float dist_receiver = shadow_calculate_distance_to_receiver(shadow_v, m);
	
	float depth = project_depth(-dist_receiver, shadow_near);//shadow_near / dist_receiver;
	float zf = shadow_calculate_test_depth(depth);
	vec3 norm_v = shadow_v / dist_receiver;
	
	float d = shadow_blocker_search(shadow_maps, idx, norm_v, m);
	float d_blocker = -unproject_depth(d, shadow_near);

	if (dist_receiver > d_blocker) {
		float penumbra = min(shadow_calculate_penumbra(d_blocker, light_radius, dist_receiver) / cutoff, max_penumbra);
		float noise = two_pi * interleaved_gradient_noise(gl_FragCoord.xy);
		
		if (penumbra >= min_penumbra) {
			float accum = .0f;
			float w = .0f;
			for (int s=0; s<8; ++s) {
				vec2 u = penumbra * shadow_cluster_samples[s];
			
				float gaussian = penumbra;
				float l = length(shadow_cluster_samples[s]) * penumbra;
				float t = exp(-l*l / (2 * gaussian * gaussian)) * one_over_pi / (2 * gaussian * gaussian);

				float shadow_sample = texture(shadow_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, u), idx)).x;
				accum += shadow_sample * t;
				w += t;
			}

			d = accum / w;
		}
	}

	float z = -unproject_depth(d, shadow_near);
	return norm_v * z;
}
