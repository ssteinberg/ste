
#type frag
#version 450

#include <material.glsl>
#include <gbuffer_store.glsl>

layout(location = 0) out vec4 gbuffer0;
layout(location = 1) out vec4 gbuffer1;

layout(std430, binding = 13) restrict readonly buffer material_data {
	material_descriptor mat_descriptor[];
};

in scene_transform {
	vec3 frag_position;
	vec3 frag_normal;
	vec3 frag_tangent;
	vec2 frag_texcoords;
	flat int matIdx;
} vin;

void main() {
	vec2 uv = vin.frag_texcoords;
	material_descriptor md = mat_descriptor[vin.matIdx];

	if (material_is_masked(md, uv)) {
		discard;
		return;
	}

	vec3 P = vin.frag_position;
	vec3 n = normalize(vin.frag_normal);
	vec3 t = normalize(vin.frag_tangent);

	int material = vin.matIdx;

	g_buffer_element gbuffer_element = gbuffer_encode(gl_FragCoord.z,
													  uv,
													  dFdx(uv),
													  dFdy(uv),
													  n,
													  t,
													  material);
	gbuffer0 = gbuffer_element.data[0];
	gbuffer1 = gbuffer_element.data[1];
}
