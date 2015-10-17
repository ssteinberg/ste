
#type compute
#version 450

#include "hdr_common.glsl"

layout(local_size_x = bins / 2, local_size_y = 1) in;

layout(std430, binding = 0) coherent buffer histogram_sums {
	uint sums[bins];
};
layout(std430, binding = 1) coherent buffer histogram_bins {
	uint histogram[bins];
};

shared uint shared_data[bins];

layout(location = 0) uniform int hdr_lum_resolution;

void main() {
	uint id = gl_LocalInvocationID.x;

/*	float N = fbins;
	float t = 1.25f / (N);
	int bin_ceil = int(hdr_lum_resolution * t);

	for (int j=0; j<2; ++j) {
		int h = int(histogram[id * 2 + j]);
		int trim = max(0, h - bin_ceil);
		shared_data[id * 2 + j] = uint(h - trim);
	}*/
	
	shared_data[id * 2] = histogram[id * 2];
	shared_data[id * 2 + 1] = histogram[id * 2 + 1];

	barrier();
	memoryBarrierShared();

	const uint steps = uint(log2(fbins) + 1);
	for (int i=0; i<steps; ++i) {
		uint mask = (1 << i) - 1;
		uint read_idx = ((id >> i) << (i + 1)) + mask;
		uint write_idx = read_idx + 1 + (id & mask);

		shared_data[write_idx] += shared_data[read_idx];

		barrier();
		memoryBarrierShared();
	}
	
	sums[id * 2] = shared_data[id * 2];
	sums[id * 2 + 1] = shared_data[id * 2 + 1];
}
