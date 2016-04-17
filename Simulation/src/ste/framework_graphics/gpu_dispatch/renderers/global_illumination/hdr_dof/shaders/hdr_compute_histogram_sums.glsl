
#type compute
#version 450
#extension GL_ARB_bindless_texture : require

#include "hdr_common.glsl"

layout(local_size_x = bins / 2, local_size_y = 1) in;

layout(bindless_sampler) uniform sampler2D z_buffer;

layout(std430, binding = 0) coherent buffer histogram_sums {
	uint sums[bins];
};
layout(std430, binding = 1) coherent buffer histogram_bins {
	uint histogram[bins];
};
layout(std430, binding = 2) coherent buffer hdr_bokeh_parameters_buffer {
	hdr_bokeh_parameters params;
};

shared uint shared_data[bins];

uniform float time;
uniform uint hdr_lum_resolution;

void main() {
	uint id = gl_LocalInvocationID.x;

	float N = fbins;
	float T = 1.25f / (N);
	int bin_ceil = int(hdr_lum_resolution * T);

	// float time_coef = 1.f / time;

	for (int j=0; j<2; ++j) {
		int h = int(histogram[id * 2 + j]);

		int trim = max(0, h - bin_ceil);
		shared_data[id * 2 + j] = uint(h - trim);
	}

	float focal;
	if (id == 0)
		focal = texelFetch(z_buffer, textureSize(z_buffer, 0) / 2, 0).x;

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

	if (id == 0) {
		float t = (focal > params.focus ? 3.2f : 13.f) * time;
		t = clamp(t, 0.f, 1.f);
		params.focus = mix(params.focus, focal, t);
	}
}
