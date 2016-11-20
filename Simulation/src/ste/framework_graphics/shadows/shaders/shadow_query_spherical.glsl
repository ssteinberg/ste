
#include "shadow_common.glsl"

const float shadow_cube_max_penumbra = 80.f / shadow_cubemap_size;
const float shadow_cube_min_penumbra = 4.f / shadow_cubemap_size;

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
float shadow(samplerCubeArrayShadow shadow_depth_maps,
			 samplerCubeArray shadow_maps, 
			 uint idx,
			 vec3 position, 
			 vec3 normal,
			 vec3 shadow_v, 
			 float light_radius,
			 ivec2 frag_coords) {
	float shadow_near = light_radius;

	mat2x3 m;
	float dist_receiver = shadow_calculate_distance_to_receiver(shadow_v, m);
	vec3 norm_v = shadow_v / dist_receiver;

	float depth_blocker = shadow_blocker_search(shadow_maps, idx, norm_v, m);
	float dt = shadow_calculate_test_depth(-dist_receiver, shadow_near);

	// No shadowing if distance to blocker is further away from receiver
	if (dt >= depth_blocker)
		return 1.f;

	float dist_blocker = -unproject_depth(depth_blocker, shadow_near);

	// Calculate penumbra based on distance and light radius
	float penumbra = clamp(shadow_calculate_penumbra(dist_blocker, light_radius, dist_receiver) / dist_receiver,
						   shadow_cube_min_penumbra,
						   shadow_cube_max_penumbra);

	// Calculate number of sampling clusters
	float clusters_to_sample = shadow_clusters_to_sample(penumbra * shadow_cubemap_size, position, normal);
	clusters_to_sample = clamp(ceil(clusters_to_sample), 1.f, shadow_max_clusters);

	// Interleaved gradient noise
	float noise = interleaved_gradient_noise(vec2(frag_coords));
	float accum = .0f;
	float w = .0f;

	// Accumulate samples
	// Each cluster is rotated around and offseted by the generated interleaved noise
	float delta = 1.f / clusters_to_sample + epsilon;
	for (float d=.0f; d<1.f; d+=delta) {
		float noise_offset = d;
		float x = two_pi * (noise + noise_offset);
		float sin_noise = sin(x);
		float cos_noise = cos(x);
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

			float shadow_sample = texture(shadow_depth_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, u), idx), dt).x;
			accum += shadow_sample * t;
			w += t;
		}

		// Stop early if first cluster is fully unshadowed
		if (d == .0f && 
			accum < 1e-12)
			break;
	}

	return smoothstep(.0, 1.f, min(1.f, accum / w / shadow_cutoff));
}

/*
 *	Fast unfiltered shadow lookup
 */
float shadow_fast(samplerCubeArrayShadow shadow_depth_maps, uint idx, vec3 shadow_v, float light_radius) {
	vec3 v = abs(shadow_v);
	float m = max_element(v);
	float shadow_near = light_radius;

	float dt = shadow_calculate_test_depth(-m, shadow_near);

	return texture(shadow_depth_maps, vec4(shadow_v, idx), dt).x;
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
vec3 shadow_occluder(samplerCubeArray shadow_maps, 
					 uint idx, 
					 vec3 shadow_v, 
					 float light_radius,
					 ivec2 frag_coords) {
	float shadow_near = light_radius;
	
	mat2x3 m;
	float dist_receiver = shadow_calculate_distance_to_receiver(shadow_v, m);
	vec3 norm_v = shadow_v / dist_receiver;

	float depth_blocker = shadow_blocker_search(shadow_maps, idx, norm_v, m);
	float dt = shadow_calculate_test_depth(-dist_receiver, shadow_near);

	// No shadowing if distance to blocker is further away from receiver
	if (dt < depth_blocker) {
		float dist_blocker = -unproject_depth(depth_blocker, shadow_near);
		float penumbra = shadow_calculate_penumbra(dist_blocker, light_radius, dist_receiver);

		float noise = two_pi * interleaved_gradient_noise(vec2(frag_coords));
		float sin_noise = sin(noise);
		float cos_noise = cos(noise);
		mat2 sample_rotation_matrix = mat2(cos_noise,  sin_noise, 
										   -sin_noise, cos_noise);
		
		if (penumbra > shadow_cube_min_penumbra) {
			penumbra = clamp(penumbra, shadow_cube_min_penumbra, shadow_cube_max_penumbra);

			float accum = .0f;
			float w = .0f;
			for (int s=0; s<8; ++s) {
				vec2 u = penumbra * sample_rotation_matrix * shadow_cluster_samples[s];
			
				float gaussian = penumbra;
				float l = length(shadow_cluster_samples[s]) * penumbra;
				float t = exp(-l*l / (2 * gaussian * gaussian)) * one_over_pi / (2 * gaussian * gaussian);

				float shadow_sample = texture(shadow_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, u), idx)).x;
				accum += shadow_sample * t;
				w += t;
			}

			depth_blocker = accum / w;
		}
	}

	float z = -unproject_depth(depth_blocker, shadow_near);
	return norm_v * z;
}
