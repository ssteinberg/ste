
#include "gbuffer.glsl"
#include "pack.glsl"

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

uint32_t gbuffer_parse_nextptr(g_buffer_element frag) {
	return floatBitsToUint(frag.data[0].y);
}

vec2 gbuffer_parse_depth_nextptr_pair(g_buffer_element frag) {
	return frag.data[0].xy;
}

float gbuffer_parse_depth(g_buffer_element frag) {
	return frag.data[0].x;
}

vec2 gbuffer_parse_uv(g_buffer_element frag) {
	return frag.data[1].xy;
}

float gbuffer_parse_alpha(g_buffer_element frag) {
	return frag.data[0].w;
}

vec3 gbuffer_parse_normal(g_buffer_element frag) {
	uint Npack = floatBitsToUint(frag.data[1].z);
	return snorm2x32_to_normal3x32(unpackSnorm2x16(Npack));
}

vec3 gbuffer_parse_tangent(g_buffer_element frag) {
	uint Tpack = floatBitsToUint(frag.data[1].w);
	return snorm2x32_to_normal3x32(unpackSnorm2x16(Tpack));
}

int gbuffer_parse_material(g_buffer_element frag) {
	return floatBitsToInt(frag.data[0].z);
}

vec2 gbuffer_parse_duvdx(g_buffer_element frag) {
	uint duvdx16 = floatBitsToUint(frag.data[2].x);
	return unpackFloat2x16(duvdx16);
}

vec2 gbuffer_parse_duvdy(g_buffer_element frag) {
	uint duvdy16 = floatBitsToUint(frag.data[2].y);
	return unpackFloat2x16(duvdy16);
}
