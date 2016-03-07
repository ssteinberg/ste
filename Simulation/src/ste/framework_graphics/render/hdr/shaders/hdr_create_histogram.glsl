
#type compute
#version 450

#include "hdr_common.glsl"

layout(local_size_x = 32, local_size_y = 32) in;

layout(r32f, binding = 0) uniform image2D hdr_lums;
layout(binding = 0) uniform atomic_uint histogram[bins];
layout(std430, binding = 2) coherent buffer hdr_bokeh_parameters_buffer {
	hdr_bokeh_parameters params;
};

void main() {
	float l = imageLoad(hdr_lums, ivec2(gl_GlobalInvocationID.xy)).x;

	float min_lum = intBitsToFloat(params.lum_min);
	float max_lum = intBitsToFloat(params.lum_max);
	
	int bin = int(hdr_bin(max_lum, min_lum, l));
	atomicCounterIncrement(histogram[bin]);
}
