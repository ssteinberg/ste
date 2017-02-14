
#type compute
#version 450
#extension GL_ARB_bindless_texture : require

#include <hdr_common.glsl>

#include <girenderer_transform_buffer.glsl>

layout(local_size_x = bins / 2, local_size_y = 1) in;

layout(shared, binding = 0) restrict writeonly buffer histogram_sums {
	uint sums[bins];
};
layout(shared, binding = 1) restrict readonly buffer histogram_bins {
	uint histogram[bins];
};
layout(std430, binding = 2) restrict writeonly buffer hdr_bokeh_parameters_buffer {
	hdr_bokeh_parameters params;
};

layout(bindless_sampler) uniform sampler2D depth_texture;

shared uint shared_data[bins];

uniform uint hdr_lum_resolution;

void main() {
	uint id = gl_LocalInvocationID.x;

	float N = fbins;
	float T = 1.25f / (N);
	int bin_ceil = int(hdr_lum_resolution * T);

	for (int j=0; j<2; ++j) {
		int h = int(histogram[id * 2 + j]);

		int trim = max(0, h - bin_ceil);
		shared_data[id * 2 + j] = uint(h - trim);
	}

	if (id == 0) {
		float d = texelFetch(depth_texture, textureSize(depth_texture, 0) >> 1, 0).x;
		float z_lin = unproject_depth(d);

		params.focus = z_lin;
	}

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
