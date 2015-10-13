
#type compute
#version 450

#include "hdr_common.glsl"

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding = 1) uniform sampler2D lum_tex;
layout(binding = 0, r32i) uniform readonly iimage2D histogram_minmax;
layout(binding = 0) uniform atomic_uint histogram[bins];

void main() {
	float l = texelFetch(lum_tex, ivec2(gl_GlobalInvocationID.xy), 0).x;

	float min_lum = intBitsToFloat(imageLoad(histogram_minmax, ivec2(0, 0)).x);
	float max_lum = intBitsToFloat(imageLoad(histogram_minmax, ivec2(1, 0)).x);
	
	int bin = int(hdr_bin(max_lum, min_lum, l));
	atomicCounterIncrement(histogram[bin]);
}
