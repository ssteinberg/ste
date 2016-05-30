
#type compute
#version 450

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding = 11) uniform sampler2D depth_target;
layout(binding = 12) uniform sampler2D downsampled_depth_target;
layout(r32f, binding = 4) restrict writeonly uniform image2D output_image;

uniform int lod;

void main() {
	ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

	float t;
	if (lod == 0) {
		float t00 = texelFetchOffset(depth_target, coords*2, 0, ivec2(0,0)).x;
		float t10 = texelFetchOffset(depth_target, coords*2, 0, ivec2(1,0)).x;
		float t01 = texelFetchOffset(depth_target, coords*2, 0, ivec2(0,1)).x;
		float t11 = texelFetchOffset(depth_target, coords*2, 0, ivec2(1,1)).x;

		t = min(min(t00,t10), min(t01,t11));
	}
	else {
		float t00 = texelFetchOffset(downsampled_depth_target, coords*2, lod - 1, ivec2(0,0)).x;
		float t10 = texelFetchOffset(downsampled_depth_target, coords*2, lod - 1, ivec2(1,0)).x;
		float t01 = texelFetchOffset(downsampled_depth_target, coords*2, lod - 1, ivec2(0,1)).x;
		float t11 = texelFetchOffset(downsampled_depth_target, coords*2, lod - 1, ivec2(1,1)).x;

		t = min(min(t00,t10), min(t01,t11));
	}

	imageStore(output_image, coords, t.xxxx);
}
