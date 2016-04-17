
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

#include "material.glsl"
#include "deferred.glsl"

in vec2 frag_texcoords;
in vec3 frag_position;
in vec3 frag_wposition;
in vec3 frag_normal;
in vec3 frag_wnormal;
in vec3 frag_tangent;
in float frag_depth;
flat in int matIdx;

uniform float height_map_scale = 1.f;

void main() {
	vec2 uv = frag_texcoords;
	material_descriptor md = mat_descriptor[matIdx];

	if (md.alphamap.tex_handler>0 && texture(sampler2D(md.alphamap.tex_handler), uv).x<.5f) {
		discard;
		return;
	}

	vec3 P = frag_position;
	vec3 w_pos = frag_wposition;
	vec3 n = normalize(frag_normal);
	vec3 w_n = normalize(frag_wnormal);
	vec3 t = normalize(frag_tangent);

	normal_map(md, height_map_scale, uv, n, t, P);

	float specular = md.specular.tex_handler>0 ? texture(sampler2D(md.specular.tex_handler), uv).x : 1.f;
	vec3 diffuse = md.diffuse.tex_handler>0 ? texture(sampler2D(md.diffuse.tex_handler), uv).rgb : vec3(1.f);

	deferred_output(P, w_pos, diffuse, specular, n, w_n, t, frag_depth, matIdx);
}
