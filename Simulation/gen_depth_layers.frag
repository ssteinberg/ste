
#version 440

layout(location = 0) out float o_d0;
layout(location = 1) out float o_d1;
layout(location = 2) out float o_d2;

layout(binding = 0, r32i) uniform iimage2D depth_layer0;
layout(binding = 1, r32i) uniform iimage2D depth_layer1;
layout(binding = 2, r32i) uniform iimage2D depth_layer2;

void main() {
	o_d0 = 1.0f - intBitsToFloat(imageLoad(depth_layer0, ivec2(gl_FragCoord.xy)).x);
	o_d1 = 1.0f - intBitsToFloat(imageLoad(depth_layer1, ivec2(gl_FragCoord.xy)).x);
	o_d2 = 1.0f - intBitsToFloat(imageLoad(depth_layer2, ivec2(gl_FragCoord.xy)).x);
}
