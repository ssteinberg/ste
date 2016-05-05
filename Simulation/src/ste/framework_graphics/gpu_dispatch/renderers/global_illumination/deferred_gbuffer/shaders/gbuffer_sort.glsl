
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 16, local_size_y = 16) in;

#include "gbuffer.glsl"

layout(shared, binding = 6) restrict buffer gbuffer_data {
	g_buffer_element gbuffer[];
};
layout(r32ui, binding = 7) restrict uniform uimage2D gbuffer_ll_heads;

#include "gbuffer_load.glsl"
#include "gbuffer_store.glsl"

const int max_depth = 6;

void main() {
	ivec2 size = gbuffer_size(gbuffer_ll_heads);
	ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	if (coords.x >= size.x ||
		coords.y >= size.y)
		return;

	uint32_t next_idx = imageLoad(gbuffer_ll_heads, coords).x;

	vec2 sorted[max_depth];

	sorted[0].x = gbuffer_parse_depth(gbuffer[next_idx]);
	sorted[0].y = uintBitsToFloat(next_idx);
	int element_count = 1;

	next_idx = gbuffer_parse_nextptr(gbuffer[next_idx]);
	bool changed_order = false;

	for (; element_count < max_depth && !gbuffer_eof(next_idx); ++element_count) {
		float z = gbuffer_parse_depth(gbuffer[next_idx]);

		int i;
		for (i = element_count; i > 0 && sorted[i - 1].x > z; --i)
			sorted[i] = sorted[i - 1];

		if (i != element_count)
			changed_order = true;

		sorted[i].x = z;
		sorted[i].y = uintBitsToFloat(next_idx);

		next_idx = gbuffer_parse_nextptr(gbuffer[next_idx]);
	}

	if (!changed_order)
		return;

	for (int i = 0; i < element_count - 1; ++i)
		gbuffer_write_nextptr(gbuffer[floatBitsToUint(sorted[i].y)], floatBitsToUint(sorted[i + 1].y));
	gbuffer_write_nextptr(gbuffer[floatBitsToUint(sorted[element_count - 1].y)], 0xFFFFFFFF);

	imageStore(gbuffer_ll_heads, coords, floatBitsToUint(sorted[0].y).xxxx);
}
