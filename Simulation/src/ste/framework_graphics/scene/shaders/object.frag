
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

layout(early_fragment_tests) in;

#include "material.glsl"
#include "gbuffer.glsl"

layout(std430, binding = 13) restrict readonly buffer material_data {
	material_descriptor mat_descriptor[];
};

layout(shared, binding = 6) restrict writeonly buffer gbuffer_data {
	g_buffer_element gbuffer[];
};

#include "gbuffer_store.glsl"

in v {
	vec3 frag_position;
	vec3 frag_normal;
	vec3 frag_tangent;
	vec2 frag_texcoords;
	flat int matIdx;
} vin;

void main() {
	vec2 uv = vin.frag_texcoords;
	material_descriptor md = mat_descriptor[vin.matIdx];

	if (material_is_masked(md, uv))
		return;

	vec3 P = vin.frag_position;
	vec3 n = normalize(vin.frag_normal);
	vec3 t = normalize(vin.frag_tangent);

	int material = vin.matIdx;

	gbuffer_store(gl_FragCoord.z,
				  uv,
				  dFdx(uv),
				  dFdy(uv),
				  n,
				  t,
				  material,
				  ivec2(gl_FragCoord.xy));
}
