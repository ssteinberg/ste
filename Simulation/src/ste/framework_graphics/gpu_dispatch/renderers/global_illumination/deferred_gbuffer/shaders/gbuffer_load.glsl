
#include "gbuffer.glsl"

bool gbuffer_eof(uint32_t ptr) {
	return ptr == 0xFFFFFFFF;
}

ivec2 gbuffer_size(layout(r32ui) restrict readonly uimage2D gbuffer_ll_heads) {
	return imageSize(gbuffer_ll_heads);
}

g_buffer_element gbuffer_load(layout(r32ui) restrict readonly uimage2D gbuffer_ll_heads, ivec2 frag_coords) {
	uint32_t idx = imageLoad(gbuffer_ll_heads, frag_coords).x;
	return gbuffer[idx];
}

g_buffer_element gbuffer_load(uint32_t ptr) {
	return gbuffer[ptr];
}

float gbuffer_linear_z(g_buffer_element frag, float far, float near = .0f) {
	return (-frag.P.z - near) / (far - near);
}
