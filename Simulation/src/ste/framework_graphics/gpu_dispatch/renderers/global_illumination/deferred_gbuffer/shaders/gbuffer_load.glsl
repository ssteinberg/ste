
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

float gbuffer_parse_depth(g_buffer_element frag) {
	return frag.data[0].x;
}

vec2 gbuffer_parse_uv(g_buffer_element frag) {
	return frag.data[0].yz;
}

uint32_t gbuffer_parse_nextptr(g_buffer_element frag) {
	return floatBitsToUint(frag.data[0].w);
}

vec3 gbuffer_parse_normal(g_buffer_element frag) {
	uint NTpack0 = floatBitsToUint(frag.data[1].x);
	uvec3 Npack = uvec3(NTpack0 & 0xFF, (NTpack0 >> 8) & 0xFF, (NTpack0 >> 16) & 0xFF);

	return oct_to_float32x3(unorm8x3_to_snorm12x2(Npack));
}

vec3 gbuffer_parse_tangent(g_buffer_element frag) {
	uint NTpack0 = floatBitsToUint(frag.data[1].x);
	uint NTpack1 = floatBitsToUint(frag.data[1].y);
	uvec3 Tpack = uvec3(NTpack1 & 0xFF, (NTpack1 >> 8) & 0xFF, (NTpack0 >> 24) & 0xFF);

	return oct_to_float32x3(unorm8x3_to_snorm12x2(Tpack));
}

int gbuffer_parse_material(g_buffer_element frag) {
	uint enc1 = floatBitsToInt(frag.data[1].y);
	return int(enc1 >> 16);
}

vec2 gbuffer_parse_duvdx(g_buffer_element frag) {
	uint duvdx16 = floatBitsToUint(frag.data[1].z);
	return unpackFloat2x16(duvdx16);
}

vec2 gbuffer_parse_duvdy(g_buffer_element frag) {
	uint duvdy16 = floatBitsToUint(frag.data[1].w);
	return unpackFloat2x16(duvdy16);
}
