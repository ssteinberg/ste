
#type compute
#version 450
#extension GL_ARB_bindless_texture : require

layout(local_size_x = 32, local_size_y = 32) in;

layout(bindless_sampler) uniform sampler2D depth_target;
layout(bindless_sampler) uniform sampler2D downsampled_depth_target;
layout(rg32f, binding = 4) restrict writeonly uniform image2D output_image;

uniform int lod;

void main() {
	ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

	float s, t;
	if (lod == 0) {
		float d00 = texelFetchOffset(depth_target, coords*2, 0, ivec2(0,0)).x;
		float d10 = texelFetchOffset(depth_target, coords*2, 0, ivec2(1,0)).x;
		float d01 = texelFetchOffset(depth_target, coords*2, 0, ivec2(0,1)).x;
		float d11 = texelFetchOffset(depth_target, coords*2, 0, ivec2(1,1)).x;
		
		t = min(min(d00,d10), min(d01,d11));
		s = max(max(d00,d10), max(d01,d11));
	}
	else {
		float d00 = texelFetchOffset(downsampled_depth_target, coords*2, lod - 1, ivec2(0,0)).x;
		float d10 = texelFetchOffset(downsampled_depth_target, coords*2, lod - 1, ivec2(1,0)).x;
		float d01 = texelFetchOffset(downsampled_depth_target, coords*2, lod - 1, ivec2(0,1)).x;
		float d11 = texelFetchOffset(downsampled_depth_target, coords*2, lod - 1, ivec2(1,1)).x;
		
		t = min(min(d00,d10), min(d01,d11));
		s = max(max(d00,d10), max(d01,d11));
	}

	imageStore(output_image, coords, vec4(t, s, 0, 0));
}
