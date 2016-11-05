
#include "common.glsl"
#include "interleaved_gradient_noise.glsl"

#include "project.glsl"

const float shadow_cubemap_size = 1024.f;
const float shadow_dirmap_size = 1024.f;
										 
const float shadow_cutoff = .5f;
const float shadow_penumbra_scale = 1.f;

const vec2 shadow_cluster_samples[8] = { vec2(-.7071f,  .7071f),
										 vec2( .0000f,	-.8750f),
										 vec2( .5303f,	 .5303f),
										 vec2(-.6250f,	-.0000f),
										 vec2( .3536f,	-.3536f),
										 vec2(-.0000f,	 .3750f),
										 vec2(-.1768f,	-.1768f),
										 vec2( .1250f,	 .0000f) };

float shadow_calculate_test_depth(float zf) {
	return zf * 1.0175f + 6.f * epsilon;
}
