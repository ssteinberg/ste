
#include "common.glsl"
#include "interleaved_gradient_noise.glsl"

#include "project.glsl"

const float shadow_cubemap_size = 1024.f;
const float shadow_dirmap_size = 1024.f;
										 
const float shadow_lookup_multiplier = 1.f + 1.5e-6;
const float shadow_lookup_delta_max = 15.f * epsilon;
const float shadow_lookup_delta_min = .0f;

const vec2 shadow_cluster_samples[8] = { vec2(-.7071f,  .7071f),
										 vec2( .0000f,	-.8750f),
										 vec2( .5303f,	 .5303f),
										 vec2(-.6250f,	-.0000f),
										 vec2( .3536f,	-.3536f),
										 vec2(-.0000f,	 .3750f),
										 vec2(-.1768f,	-.1768f),
										 vec2( .1250f,	 .0000f) };

float shadow_calculate_test_depth(float zf) {
	float x = log(1.f + zf*(e-1));
	//float delta = mix(shadow_lookup_delta_min, shadow_lookup_delta_max, x);
	//return zf * shadow_lookup_multiplier + delta;
	return zf * 1.0175f + 8.f * epsilon;
}

float shadow_calculate_penumbra(float d_blocker, float radius, float dist_receiver) {
	const float penumbra_scale = 1.f;

	float w_penumbra = (dist_receiver - d_blocker) * radius / d_blocker;
	return penumbra_scale * w_penumbra / dist_receiver;
}
