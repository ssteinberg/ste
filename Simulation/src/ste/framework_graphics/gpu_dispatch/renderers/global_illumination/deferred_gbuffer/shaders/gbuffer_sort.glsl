
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 32, local_size_y = 32) in;

#include "gbuffer.glsl"

layout(shared, binding = 6) restrict buffer gbuffer_data {
	g_buffer_element gbuffer[];
};
layout(r32ui, binding = 7) restrict uniform uimage2D gbuffer_ll_heads;

#include "gbuffer_load.glsl"
#include "gbuffer_store.glsl"

const int max_depth = 6;

struct fragment {
	float z;
	uint ptr;
};

void main() {
	ivec2 size = gbuffer_size(gbuffer_ll_heads);
	ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	if (coords.x >= size.x ||
		coords.y >= size.y)
		return;

	uint32_t head_idx = imageLoad(gbuffer_ll_heads, coords).x;
	vec2 z_next_pair = gbuffer_parse_depth_nextptr_pair(gbuffer[head_idx]);

	fragment sorted[max_depth];

	sorted[0].z = z_next_pair.x;
	sorted[0].ptr = head_idx;
	int element_count = 1;

	uint32_t next_idx = floatBitsToUint(z_next_pair.y);
	bool changed_order = false;

	for (; element_count < max_depth && !gbuffer_eof(next_idx); ++element_count) {
		vec2 z_next_pair = gbuffer_parse_depth_nextptr_pair(gbuffer[next_idx]);

		int i;
		for (i = element_count; i > 0 && sorted[i - 1].z < z_next_pair.x; --i)
			sorted[i] = sorted[i - 1];

		if (i != element_count)
			changed_order = true;

		sorted[i].z = z_next_pair.x;
		sorted[i].ptr = next_idx;

		next_idx = floatBitsToUint(z_next_pair.y);
	}

	if (!changed_order)
		return;

	for (int i = 0; i < element_count - 1; ++i)
		gbuffer_write_nextptr(gbuffer[sorted[i].ptr], sorted[i + 1].ptr);
	gbuffer_write_nextptr(gbuffer[sorted[element_count - 1].ptr], 0xFFFFFFFF);

	if (head_idx != sorted[0].ptr)
		imageStore(gbuffer_ll_heads, coords, sorted[0].ptr.xxxx);
}
