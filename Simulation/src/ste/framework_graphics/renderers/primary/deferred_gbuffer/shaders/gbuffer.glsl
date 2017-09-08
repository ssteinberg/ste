
#include <pack.glsl>

struct g_buffer_element {
	mat2x4 data;
};

struct gbuffer_fragment_information {
	float depth;
	vec2 uv;
	vec2 duvdx, duvdy;
	vec3 n;
	vec3 t;
	vec3 b;
	int mat;
};

layout(set=2, binding=19) uniform sampler2D downsampled_depth_map;
layout(set=2, binding=20) uniform sampler2D depth_map;
layout(set=2, binding=21) uniform sampler2D backface_depth_map;
layout(set=2, binding=22) uniform sampler2DArray gbuffer;

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

gbuffer_fragment_information gbuffer_parse_fragment_information(g_buffer_element frag) {
	gbuffer_fragment_information info;
	info.depth = gbuffer_parse_depth(frag);
	info.uv = gbuffer_parse_uv(frag);
	info.duvdx = gbuffer_parse_duvdx(frag);
	info.duvdy = gbuffer_parse_duvdy(frag);
	info.n = gbuffer_parse_normal(frag);
	info.t = gbuffer_parse_tangent(frag);
	info.b = cross(info.t, info.n);
	info.mat = gbuffer_parse_material(frag);

	return info;
}
