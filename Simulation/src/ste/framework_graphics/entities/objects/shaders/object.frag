
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

layout(early_fragment_tests) in;

#include "material.glsl"
#include "gbuffer.glsl"

layout(std430, binding = 0) restrict readonly buffer material_data {
	material_descriptor mat_descriptor[];
};

layout(shared, binding = 6) restrict writeonly buffer gbuffer_data {
	g_buffer_element gbuffer[];
};
layout(binding = 7) uniform atomic_uint gbuffer_ll_counter;
layout(r32ui, binding = 7) restrict uniform uimage2D gbuffer_ll_heads;

#include "gbuffer_store.glsl"

in v {
	vec3 frag_position;
	vec2 frag_texcoords;
	vec3 frag_normal;
	vec3 frag_tangent;
	flat int matIdx;
} vin;
uniform float height_map_scale = .5f;

void main() {
	vec2 uv = vin.frag_texcoords;
	material_descriptor md = mat_descriptor[vin.matIdx];

	float alpha = md.alphamap.tex_handler>0 ? texture(sampler2D(md.alphamap.tex_handler), uv).x : 1.f;
	if (alpha == .0f)
		return;

	vec3 P = vin.frag_position;
	vec3 n = normalize(vin.frag_normal);
	vec3 t = normalize(vin.frag_tangent);
	vec3 b = cross(t, n);

	normal_map(md, height_map_scale, uv, n, t, b, P);

	int material = vin.matIdx >= 0 ? vin.matIdx : material_none;

	gbuffer_store(gbuffer_ll_heads, gbuffer_ll_counter, gl_FragCoord.z, uv, f16vec3(n), f16vec3(t), material, ivec2(gl_FragCoord.xy));
}
