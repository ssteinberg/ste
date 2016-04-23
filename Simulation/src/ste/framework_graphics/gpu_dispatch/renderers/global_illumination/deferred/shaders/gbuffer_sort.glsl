
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 16, local_size_y = 16) in;

#include "gbuffer.glsl"

const int max_depth = 16;

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

	uint32_t head = imageLoad(gbuffer_ll_heads, coords).x;

	fragment sorted[max_depth];
	int element_count = 1;

	sorted[0].z = gbuffer[head].P.z;
	sorted[0].idx = head;

	uint32_t next_idx = sorted[0].idx;

	while (!gbuffer_eof(next_idx) && element_count < max_depth) {
		float z = gbuffer[next_idx].P.z;
		uint32_t idx = next_idx;

		next_idx = gbuffer[next_idx].next_ptr;

		int i;
		for (i = element_count; i > 0 && sorted[i - 1].z < z; --i);
		for (int j = element_count; j > i; --j)
			sorted[j] = sorted[j - 1];

		sorted[i].z = z;
		sorted[i].idx = idx;

		++element_count;
	}

	for (int i = 0; i < element_count - 1; ++i)
		gbuffer[sorted[i].idx].next_ptr = sorted[i + 1].idx;
	gbuffer[element_count - 1].next_ptr = 0xFFFFFFFF;

	imageStore(gbuffer_ll_heads, coords, sorted[0].idx.xxxx);
}
