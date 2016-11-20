
#include "shadow_common.glsl"
#include "light_cascades.glsl"

const float shadow_directional_max_penumbra = 10.f;
const float shadow_directional_min_penumbra = 1.f;

float shadow_blocker_search(sampler2DArray directional_shadow_maps, uint idx, vec2 uv) {
	float d = texture(directional_shadow_maps, vec3(uv, idx)).x;
	return d;
}

/*
 *	Calculate percentage of unshadowed light irradiating in direction and distance given by 'shadow_v' from light.
 *	Filters shadow maps using samples from an interleaved gradient noise pattern, based on penumbra size.
 */
float shadow(sampler2DArrayShadow directional_shadow_depth_maps,
			 sampler2DArray directional_shadow_maps,
			 uint idx,
			 vec3 position,
			 vec3 normal,
			 mat3x4 cascade_transform,
			 vec2 cascade_recp_vp,
			 float light_distance,
			 float light_radius,
			 ivec2 frag_coords) {
	vec3 v = vec4(position, 1) * cascade_transform;
	vec2 uv = v.xy * .5f + vec2(.5f);
	float cascade_space_dist_receiver = -v.z;

	float depth_blocker = shadow_blocker_search(directional_shadow_maps, idx, uv);
	float dt = shadow_calculate_test_depth(v.z, cascade_proj_near_clip);

	// No shadowing if distance to blocker is further away from receiver
	if (dt >= depth_blocker)
		return 1.f;
		
	float cascade_space_d_blocker = -unproject_depth(depth_blocker, cascade_proj_near_clip);

	// For directional lights, the distance to the receiver is constant. Based on that, adjust the blocker distance.
	float dist_receiver = light_distance;
	float d_blocker = light_distance - (cascade_space_dist_receiver - cascade_space_d_blocker);

	// Calculate penumbra based on distance and light radius
	float penumbra_world_space = clamp(shadow_calculate_penumbra(d_blocker, light_radius, dist_receiver),
									   shadow_directional_min_penumbra,
									   shadow_directional_max_penumbra);
	
	// Calculate number of sampling clusters anisotropically
	float clusters_to_sample = shadow_clusters_to_sample(penumbra_world_space * shadow_dirmap_size, position, normal);
	clusters_to_sample = clamp(ceil(clusters_to_sample), 1.f, shadow_max_clusters); 

	// Transform penumbra size to cascade texture space (enforce at least 1 texel radius filtering to avoid aliasing)
	vec2 to_texture_space = .25f * cascade_recp_vp;
	const vec2 minimal_texture_space_penumbra = vec2(1.f / shadow_dirmap_size);
	vec2 penumbra = max(penumbra_world_space * to_texture_space, minimal_texture_space_penumbra);
	
	// Interleaved gradient noise
	float noise = interleaved_gradient_noise(vec2(frag_coords));
	float accum = .0f;
	float w = .0f;

	// Accumulate samples
	// Each cluster is rotated around and offseted by the generated interleaved noise
	float delta = 1.f / clusters_to_sample + epsilon;
	int i=0;
	for (float d=.0f; d<1.f; d+=delta, ++i) {
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
			
			float gaussian = mix(penumbra.x, penumbra.y, .5f);
			float l = length(shadow_cluster_samples[s] * penumbra);
			float t = exp(-l*l / (2 * gaussian * gaussian)) * one_over_pi / (2 * gaussian * gaussian);

			float shadow_sample = texture(directional_shadow_depth_maps, vec4(uv + u, idx, dt)).x;
			accum += shadow_sample * t;
			w += t;
		}
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

	float dt = shadow_calculate_test_depth(v.z, cascade_proj_near_clip);

	return texture(directional_shadow_depth_maps, vec4(uv, idx, dt)).x;
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
					 float light_radius,
					 ivec2 frag_coords) {
	vec3 v = vec4(position, 1) * cascade_transform;
	vec2 uv = v.xy * .5f + vec2(.5f);
	float cascade_space_dist_receiver = -v.z;

	float depth_blocker = shadow_blocker_search(directional_shadow_maps, idx, uv);
	float dt = shadow_calculate_test_depth(v.z, cascade_proj_near_clip);
	
	// No shadowing if distance to blocker is further away from receiver
	if (dt < depth_blocker) {
		float dist_receiver = light_distance;
		float dist_blocker = -unproject_depth(depth_blocker, cascade_proj_near_clip);
		float penumbra = shadow_calculate_penumbra(dist_blocker, light_radius, dist_receiver);
		
		float noise = two_pi * interleaved_gradient_noise(vec2(frag_coords));
		float sin_noise = sin(noise);
		float cos_noise = cos(noise);
		mat2 sample_rotation_matrix = mat2(cos_noise,  sin_noise, 
										   -sin_noise, cos_noise);
		
		if (penumbra > shadow_directional_min_penumbra) {
			penumbra = clamp(penumbra, shadow_directional_min_penumbra, shadow_directional_max_penumbra);

			float accum = .0f;
			float w = .0f;
			for (int s=0; s<8; ++s) {
				vec2 u = penumbra * sample_rotation_matrix * shadow_cluster_samples[s];
			
				float gaussian = penumbra;
				float l = length(shadow_cluster_samples[s]) * penumbra;
				float t = exp(-l*l / (2 * gaussian * gaussian)) * one_over_pi / (2 * gaussian * gaussian);

				float shadow_sample = texture(directional_shadow_maps, vec3(uv + u, idx)).x;
				accum += shadow_sample * t;
				w += t;
			}

			depth_blocker = accum / w;
		}
	}

	float z = -unproject_depth(depth_blocker, cascade_proj_near_clip);
	return -l * z;
}
