
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 16, local_size_y = 16) in;

#include "gbuffer.glsl"

const int max_depth = 4;

struct fragment {
	float z;
	uint32_t idx;
};

void main() {
	ivec2 size = gbuffer_size();
	ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	if (coords.x >= size.x ||
		coords.y >= size.y)
		return;

	uint32_t next_idx = imageLoad(gbuffer_ll_heads, coords).x;

	fragment sorted[max_depth];

	sorted[0].z = gbuffer[next_idx].P.z;
	sorted[0].idx = next_idx;
	int element_count = 1;

	next_idx = gbuffer[next_idx].next_ptr;
	bool changed_order = false;

	for (; element_count < max_depth && !gbuffer_eof(next_idx); ++element_count) {
		float z = gbuffer[next_idx].P.z;

		int i;
		for (i = element_count; i > 0 && sorted[i - 1].z < z; --i)
			sorted[i] = sorted[i - 1];

		if (i != element_count)
			changed_order = true;

		sorted[i].z = z;
		sorted[i].idx = next_idx;

		next_idx = gbuffer[next_idx].next_ptr;
	}

	if (!changed_order)
		return;

	for (int i = 0; i < element_count - 1; ++i)
		gbuffer[sorted[i].idx].next_ptr = sorted[i + 1].idx;
	gbuffer[sorted[element_count - 1].idx].next_ptr = 0xFFFFFFFF;

	imageStore(gbuffer_ll_heads, coords, sorted[0].idx.xxxx);
}
