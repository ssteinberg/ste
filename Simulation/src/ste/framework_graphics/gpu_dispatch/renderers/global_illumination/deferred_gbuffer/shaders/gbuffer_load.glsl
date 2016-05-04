
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

float gbuffer_parse_alpha(g_buffer_element frag) {
	return frag.data2.z;
}

vec3 gbuffer_parse_position(g_buffer_element frag) {
	return frag.data0.xyz;
}

vec2 gbuffer_parse_uv(g_buffer_element frag) {
	return frag.data2.xy;
}

float gbuffer_parse_specular(g_buffer_element frag) {
	return frag.data2.w;
}

vec3 gbuffer_parse_normal(g_buffer_element frag) {
	float pNxy = frag.data1.x;
	float pNzTx = frag.data1.y;

	vec2 Nxy = unpackFloat2x16(floatBitsToUint(pNxy));
	vec2 NzTx = unpackFloat2x16(floatBitsToUint(pNzTx));

	return vec3(Nxy, NzTx.x);
}

int gbuffer_parse_material(g_buffer_element frag) {
	return floatBitsToInt(frag.data1.w);
}

vec3 gbuffer_parse_tangent(g_buffer_element frag) {
	float pNzTx = frag.data1.y;
	float pTyz = frag.data1.z;

	vec2 NzTx = unpackFloat2x16(floatBitsToUint(pNzTx));
	vec2 Tyz = unpackFloat2x16(floatBitsToUint(pTyz));

	return vec3(NzTx.y, Tyz);
}

uint32_t gbuffer_parse_nextptr(g_buffer_element frag) {
	return floatBitsToUint(frag.data0.w);
}

float gbuffer_linear_z(g_buffer_element frag, float far, float near = .0f) {
	return (-gbuffer_parse_position(frag).z - near) / (far - near);
}
