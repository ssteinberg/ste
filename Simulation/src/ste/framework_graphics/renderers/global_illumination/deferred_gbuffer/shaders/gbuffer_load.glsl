
#include "gbuffer.glsl"
#include "pack.glsl"
#include "girenderer_transform_buffer.glsl"

bool gbuffer_eof(uint ptr) {
	return ptr == 0xFFFFFFFF;
}

g_buffer_element gbuffer_load(ivec2 frag_coords) {
	uint idx = frag_coords.y * backbuffer_size().x + frag_coords.x;
	return gbuffer[idx];
}

float gbuffer_parse_depth(g_buffer_element frag) {
	return frag.data[0].x;
}

vec2 gbuffer_parse_uv(g_buffer_element frag) {
	return frag.data[1].xy;
}

vec3 gbuffer_parse_normal(g_buffer_element frag) {
	uint Npack = floatBitsToUint(frag.data[1].z);
	return snorm2x32_to_norm3x32(unpackSnorm2x16(Npack));
}

vec3 gbuffer_parse_tangent(g_buffer_element frag) {
	uint Tpack = floatBitsToUint(frag.data[1].w);
	return snorm2x32_to_norm3x32(unpackSnorm2x16(Tpack));
}

int gbuffer_parse_material(g_buffer_element frag) {
	return floatBitsToInt(frag.data[0].y);
}

vec2 gbuffer_parse_duvdx(g_buffer_element frag) {
	uint duvdx16 = floatBitsToUint(frag.data[0].z);
	return unpackHalf2x16(duvdx16);
}

vec2 gbuffer_parse_duvdy(g_buffer_element frag) {
	uint duvdy16 = floatBitsToUint(frag.data[0].w);
	return unpackHalf2x16(duvdy16);
}
