
#type frag
#version 450
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

#include "material.glsl"

in vec2 frag_texcoords;
in vec3 frag_position;
in vec3 frag_normal;
in vec3 frag_tangent;
in float frag_depth;
flat in int drawId;

layout(location = 0) out vec3 o_frag_position;
layout(location = 1) out vec2 o_frag_uv;
layout(location = 2) out vec4 o_frag_duv;
layout(location = 3) out vec3 o_frag_normal;
layout(location = 4) out float o_frag_z;
layout(location = 5) out vec3 o_frag_tangent;
layout(location = 6) out int o_frag_mat_idx;

uniform float height_map_scale = 1.f;

layout(std430, binding = 0) buffer material_data {
	material_descriptor mat_descriptor[];
};

void main() {
	material_descriptor md = mat_descriptor[drawId];

	if (md.alphamap.tex_handler>0 && texture(sampler2D(md.alphamap.tex_handler), frag_texcoords).x<.5f) {
		discard;
		return;
	}
	
	vec2 uv = frag_texcoords;
	vec3 P = frag_position;
	vec3 n = normalize(frag_normal);
	vec3 t = normalize(frag_tangent);
	
	o_frag_tangent = t;
	o_frag_normal = n;
	o_frag_uv = uv;
	o_frag_duv = vec4(dFdx(uv), dFdy(uv));

	o_frag_position = P;
	o_frag_z = frag_depth;
	o_frag_mat_idx = drawId;
}
