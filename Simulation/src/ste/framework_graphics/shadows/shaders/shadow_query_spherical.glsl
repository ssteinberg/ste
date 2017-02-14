
#include <shadow_common.glsl>
#include <deferred_shading_common.glsl>

const float shadow_cube_max_penumbra = 80.f / shadow_cubemap_size;
const float shadow_cube_min_penumbra = 3.f / shadow_cubemap_size;

/*
 *	Creates a slightly offseted testing depth for shadowmaps lookups
 */
float shadow_calculate_test_depth(float z, float near, float far, vec3 n, vec3 l) {
	float d = project_depth(z, near);//project_depth_linear(z, near, far);

	float slope = 1.f - abs(dot(l,n));

	float df_dx = near / sqr(z);
	float delta = df_dx * 1.1f * (1.f + slope * 2.5f);
	return d + delta;
}

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
 *	Shadow lookup and filtering implementation for spherical lights.
 */
float shadow_impl(deferred_shading_shadow_maps shadow_maps, 
				  uint idx,
				  vec3 position, 
				  vec3 l,
				  vec3 normal,
				  vec3 shadow_v, 
				  float light_near_clip,
				  float light_effective_range,
				  ivec2 frag_coords,
				  float max_penumbra_multiplier,		// Modulates the maximal allowed penumbra
				  bool override_clusters,				// Overrides the calculated clusters amount
				  int override_clusters_amount) {
	float shadow_near = light_near_clip * 2;

	mat2x3 m;
	float dist_receiver = shadow_calculate_distance_to_receiver(shadow_v, m);
	vec3 norm_v = shadow_v / dist_receiver;

	float depth_blocker = shadow_blocker_search(shadow_maps.shadow_maps, idx, norm_v, m);
	float dt = shadow_calculate_test_depth(-dist_receiver, shadow_near, light_effective_range, normal, l);

	// No shadowing if distance to blocker is further away from receiver
	if (dt >= depth_blocker)
		return 1.f;

	float dist_blocker = -unproject_depth(depth_blocker, shadow_near);//-unproject_depth_linear(depth_blocker, shadow_near, light_effective_range);

	// Calculate penumbra based on distance and light radius
	float penumbra_world = shadow_calculate_penumbra(dist_blocker, shadow_near, dist_receiver);
	float penumbra = clamp(penumbra_world / dist_receiver,
						   shadow_cube_min_penumbra,
						   shadow_cube_max_penumbra * max_penumbra_multiplier);

	// Calculate number of sampling clusters
	float clusters = shadow_clusters_to_sample(penumbra_world, position, normal, l);
	int clusters_to_sample = min(int(ceil(clusters)), shadow_max_clusters);
	if (override_clusters)
		clusters_to_sample = override_clusters_amount;

	// Interleaved gradient noise
	float noise = interleaved_gradient_noise(vec2(frag_coords));
	float rcp = 1.f / float(clusters_to_sample);
	
	float accum = texture(shadow_maps.shadow_depth_maps, vec4(norm_v, idx), dt).x;
	float w = float(clusters_to_sample) * 8.f + 1.f;

	// Accumulate samples
	// Each cluster is rotated around and offseted by the generated interleaved noise
	for (int i=0; i<clusters_to_sample; ++i) {
		float noise_offset = float(i) * rcp;
		float x = two_pi * (noise + noise_offset);
		float sin_noise = sin(x);
		float cos_noise = cos(x);
		mat2 sample_rotation_matrix = mat2(cos_noise,  sin_noise, 
										   -sin_noise, cos_noise);

		// Constant sample pattern per cluster
		// Calculate the offset, lookup shadowmap texture
		for (int s=0; s<8; ++s) {
			vec2 u;
			u = penumbra * (sample_rotation_matrix * shadow_cluster_samples[s]);
			accum += texture(shadow_maps.shadow_depth_maps, vec4(shadow_cubemap_jitter_uv(norm_v, m, u), idx), dt).x;
		}
	}

	return min(1.f, (accum / w) / shadow_cutoff);
}

/*
 *	Calculate percentage of unshadowed light irradiating in direction and distance given by 'shadow_v' from light.
 *	Filters shadow maps using samples from an interleaved gradient noise pattern, based on penumbra size.
 */
float shadow(deferred_shading_shadow_maps shadow_maps, 
			 uint idx,
			 vec3 position, 
			 vec3 l,
			 vec3 normal,
			 vec3 shadow_v, 
			 float light_near_clip,
			 float light_effective_range,
			 ivec2 frag_coords) {
	return shadow_impl(shadow_maps, 
					   idx,
					   position, 
					   l,
					   normal,
					   shadow_v, 
					   light_near_clip,
					   light_effective_range,
					   frag_coords,
					   1.f,
					   false,
					   0);
}

/*
 *	Calculate percentage of unshadowed light irradiating in direction and distance given by 'shadow_v' from light.
 *	Filters shadow maps using samples from an interleaved gradient noise pattern, based on penumbra size.
 *	Fast version, filtering is limited to a single cluster with reduced penumbras.
 */
float shadow_fast(deferred_shading_shadow_maps shadow_maps, 
				  uint idx,
				  vec3 position, 
				  vec3 l,
				  vec3 normal,
				  vec3 shadow_v, 
				  float light_near_clip,
				  float light_effective_range,
				  ivec2 frag_coords) {
	return shadow_impl(shadow_maps, 
					   idx,
					   position,
					   l,
					   normal,
					   shadow_v, 
					   light_near_clip,
					   light_effective_range,
					   frag_coords,
					   .25f,
					   true,
					   1);
}

/*
 *	Fast unfiltered shadow lookup
 */
float shadow_test(samplerCubeArrayShadow shadow_depth_maps, uint idx, vec3 shadow_v, 
				  vec3 l, vec3 normal,
				  float light_near_clip, float light_effective_range) {
	vec3 v = abs(shadow_v);
	float m = max_element(v);
	float shadow_near = light_near_clip * 2;

	float dt = shadow_calculate_test_depth(-m, shadow_near, light_effective_range, normal, l);

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
					 vec3 l,
					 vec3 normal,
					 float light_near_clip,
					 float light_effective_range,
					 ivec2 frag_coords) {
	float shadow_near = light_near_clip * 2;
	
	mat2x3 m;
	float dist_receiver = shadow_calculate_distance_to_receiver(shadow_v, m);
	vec3 norm_v = shadow_v / dist_receiver;

	float depth_blocker = shadow_blocker_search(shadow_maps, idx, norm_v, m);
	float dt = shadow_calculate_test_depth(-dist_receiver, shadow_near, light_effective_range, normal, l);

	// No shadowing if distance to blocker is further away from receiver
	if (dt < depth_blocker) {
		float dist_blocker = -unproject_depth(depth_blocker, shadow_near);//-unproject_depth_linear(depth_blocker, shadow_near, light_effective_range);
		float penumbra = shadow_calculate_penumbra(dist_blocker, shadow_near, dist_receiver) / dist_receiver;

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

	float z = -unproject_depth(depth_blocker, shadow_near);//-unproject_depth_linear(depth_blocker, shadow_near, light_effective_range);
	return norm_v * z;
}
