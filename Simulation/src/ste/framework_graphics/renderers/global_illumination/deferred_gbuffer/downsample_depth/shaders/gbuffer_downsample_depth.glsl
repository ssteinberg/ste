
#type compute
#version 450
#extension GL_ARB_bindless_texture : require

layout(local_size_x = 32, local_size_y = 32) in;

#include "common.glsl"

layout(bindless_sampler) uniform sampler2D depth_target;
layout(bindless_sampler) uniform sampler2D downsampled_depth_target;
layout(rg32f, binding = 4) restrict writeonly uniform image2D output_image;

uniform int lod;

void main() {
	ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

	float s, t;
	if (lod == 0) {
		vec4 d = vec4(texelFetchOffset(depth_target, coords*2, 0, ivec2(0,0)).x,
					  texelFetchOffset(depth_target, coords*2, 0, ivec2(1,0)).x,
					  texelFetchOffset(depth_target, coords*2, 0, ivec2(0,1)).x,
					  texelFetchOffset(depth_target, coords*2, 0, ivec2(1,1)).x);
		
		bvec4 b = equal(d, vec4(0));
		vec4 mind = mix(d, vec4(1), b);
		
		t = min_element(mind);
		s = max_element(d);
	}
	else {
		vec4 d = vec4(texelFetchOffset(downsampled_depth_target, coords*2, lod - 1, ivec2(0,0)).x,
					  texelFetchOffset(downsampled_depth_target, coords*2, lod - 1, ivec2(1,0)).x,
					  texelFetchOffset(downsampled_depth_target, coords*2, lod - 1, ivec2(0,1)).x,
					  texelFetchOffset(downsampled_depth_target, coords*2, lod - 1, ivec2(1,1)).x);
		
		bvec4 b = equal(d, vec4(0));
		vec4 mind = mix(d, vec4(1), b);
		
		t = min_element(mind);
		s = max_element(d);
	}

	imageStore(output_image, coords, vec4(t, s, 0, 0));
}
