
#include "shadow_common.glsl"
#include "light_cascades.glsl"

float shadow_blocker_search(sampler2DArray directional_shadow_maps, uint idx, vec2 uv) {
	float d = texture(directional_shadow_maps, vec3(uv, idx)).x;
	return d;
}

vec2 shadow_calculate_penumbra_directional(float d_blocker, float radius, float dist_receiver, vec2 recp_vp, out float penumbra_ratio) {
	const float max_penumbra = 10.f;
	const float min_penumbra = 1.f;
	
	float w_penumbra = (dist_receiver - d_blocker) / d_blocker * radius;
	float penumbra = clamp(shadow_penumbra_scale * w_penumbra / shadow_cutoff,
						   min_penumbra,
						   max_penumbra);

	penumbra_ratio = (penumbra - min_penumbra) / (max_penumbra - min_penumbra);

	vec2 to_texture_space = .25f * recp_vp;
	return penumbra * to_texture_space;
}

/*
 *	Calculate percentage of unshadowed light irradiating in direction and distance given by 'shadow_v' from light.
 *	Filters shadow maps using samples from an interleaved gradient noise pattern, based on penumbra size.
 */
float shadow(sampler2DArrayShadow directional_shadow_depth_maps,
			 sampler2DArray directional_shadow_maps,
			 uint idx, 
			 vec3 position, 
			 mat3x4 cascade_transform,
			 vec2 cascade_recp_vp,
			 float light_distance,
			 float light_radius) {
	const float penumbra_w_to_clusters_ratio_power = 1.3f;
	const float max_cluster_samples = 4.f;

	vec3 v = vec4(position, 1) * cascade_transform;
	vec2 uv = v.xy * .5f + vec2(.5f);
	float cascade_space_dist_receiver = -v.z;

	float d = shadow_blocker_search(directional_shadow_maps, idx, uv);
	float cascade_space_d_blocker = -unproject_depth(d, cascade_proj_near_clip);

	// No shadowing if distance to blocker is further away from receiver
	if (cascade_space_dist_receiver <= cascade_space_d_blocker)
		return 1.f;

	// For directional lights, the distance to the receiver is constant. Based on that, adjust the blocker distance.
	float dist_receiver = light_distance;
	float d_blocker = light_distance - (cascade_space_dist_receiver - cascade_space_d_blocker);

	float depth = project_depth(v.z, cascade_proj_near_clip);
	float zf = shadow_calculate_test_depth(depth);

	// Calculate penumbra based on distance and light radius
	float penumbra_ratio;
	vec2 penumbra = shadow_calculate_penumbra_directional(d_blocker, light_radius, dist_receiver, cascade_recp_vp, penumbra_ratio);

	// Number of clusters to sample calculated heuristically using the penumbra size
	float clusters_ratio = pow(penumbra_ratio, penumbra_w_to_clusters_ratio_power);
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
		mat2 sample_rotation_matrix = mat2(cos_noise, sin_noise, -sin_noise, cos_noise);

		// Constant sample pattern per cluster
		// Calculate the offset, lookup shadowmap texture
		// Weights per sample are Gaussian
		for (int s=0; s<8; ++s) {
			vec2 u = penumbra * (sample_rotation_matrix * shadow_cluster_samples[s]);
			
			float gaussian = mix(penumbra.x, penumbra.y, .5f);
			float l = length(shadow_cluster_samples[s] * penumbra);
			float t = exp(-l*l / (2 * gaussian * gaussian)) * one_over_pi / (2 * gaussian * gaussian);

			float shadow_sample = texture(directional_shadow_depth_maps, vec4(uv + u, idx, zf)).x;
			accum += shadow_sample * t;
			w += t;
		}

		// Stop early if first cluster is fully unshadowed
		/*if (i == 2 && 
			accum < epsilon)
			break;*/
	}

	return smoothstep(.0, 1.f, min(1.f, accum / w / shadow_cutoff));
}

/*
 *	Fast unfiltered shadow lookup
 */
float shadow_fast(sampler2DArrayShadow directional_shadow_depth_maps, 
				  uint idx, 
				  vec3 position, 
				  mat3x4 cascade_transform) {
	vec3 v = vec4(position, 1) * cascade_transform;
	vec2 uv = v.xy * .5f + vec2(.5f);
	float zf = project_depth(v.z, cascade_proj_near_clip);
	zf = shadow_calculate_test_depth(zf);

	return texture(directional_shadow_depth_maps, vec4(uv, idx, zf)).x;
}

/*
 *	Lookup unfiltered depth from shadowmap
 */
float shadow_depth(sampler2DArray directional_shadow_maps, uint idx, vec3 position, mat3x4 cascade_transform) {
	vec3 v = vec4(position, 1) * cascade_transform;
	vec2 uv = v.xy * .5f + vec2(.5f);
	return texture(directional_shadow_maps, vec3(uv, idx)).x;
}

/*
 *	Lookup shadow occluder in shadow_v direction
 *	Filtered, like 'shadow' method but limited to a single cluster and smaller upper limit on penumbra size
 */
vec3 shadow_occluder(sampler2DArray directional_shadow_maps, 
					 uint idx, 
					 vec3 position, 
					 mat3x4 cascade_transform,
					 vec3 l,
					 float light_distance,
					 float light_radius) {return vec3(0);/*
	const float cutoff = .5f;
	const float max_penumbra = 10.f / shadow_cubemap_size;
	const float min_penumbra = 4.f / shadow_cubemap_size;

	mat2x3 m;
	float dist_receiver = light_distance;
	
	vec3 v = vec4(position, 1) * cascade_transform;
	vec2 uv = v.xy * .5f + vec2(.5f);

	float depth = project_depth(v.z, cascade_proj_near_clip);
	float zf = shadow_calculate_test_depth(depth);
	
	float d = shadow_blocker_search(directional_shadow_maps, idx, uv);
	float d_blocker = -unproject_depth(d, cascade_proj_near_clip);

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

				float shadow_sample = texture(directional_shadow_maps, vec3(uv + u, idx)).x;
				accum += shadow_sample * t;
				w += t;
			}

			d = accum / w;
		}
	}

	float z = -unproject_depth(d, cascade_proj_near_clip);
	return -l * z;*/
}
