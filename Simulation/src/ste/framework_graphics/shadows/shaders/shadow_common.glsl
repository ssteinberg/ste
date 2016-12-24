
#include "common.glsl"
#include "interleaved_gradient_noise.glsl"

#include "girenderer_transform_buffer.glsl"
#include "project.glsl"

const float shadow_cubemap_size = 1024.f;
const float shadow_dirmap_size = 2048.f;

const float shadow_max_penumbra_fast_path_multiplier = .25f;

const int shadow_max_clusters = 10;
const float shadow_cutoff = .5f;
const float shadow_penumbra_scale = 1.f;
const float shadow_screen_penumbra_clusters_per_pixel_squared = 2e-4f;

const vec2 shadow_cluster_samples[8] = { vec2(-.7071f,   .7071f),
										 vec2( .0000f,	-.8750f),
										 vec2( .5303f,	 .5303f),
										 vec2(-.6250f,	-.0000f),
										 vec2( .3536f,	-.3536f),
										 vec2(-.0000f,	 .3750f),
										 vec2(-.1768f,	-.1768f),
										 vec2( .1250f,	 .0000f) };

/*
 *	Calculate shadow penumbra size
 */
float shadow_calculate_penumbra(float d_blocker, float radius, float dist_receiver) {
	float w_penumbra = (dist_receiver - d_blocker) / d_blocker * radius;
	return shadow_penumbra_scale * w_penumbra / shadow_cutoff;
}

/*
 *	Calculate the screen-space anisotropic penumbra and based on that returns the number of clusters to sample
 */
float shadow_clusters_to_sample(float penumbra, vec3 position, vec3 normal) {
	// Project penumbra to NDC
	vec2 ansi = vec2(1) - abs(normal.xy);
	vec2 ansi_penumbra = ansi * penumbra;
	vec4 penumbra_ndc = project(vec3(ansi_penumbra, position.z));

	// Convert penumbra xy from NDC size to screen pixels
	vec2 projected = penumbra_ndc.xy / penumbra_ndc.w;
	vec2 screen_space_penumbra_pixels = projected * vec2(backbuffer_size());

	// Calculate the ellipsis area
	float pixels = pi * screen_space_penumbra_pixels.x * screen_space_penumbra_pixels.y;

	return pixels * shadow_screen_penumbra_clusters_per_pixel_squared;
}
