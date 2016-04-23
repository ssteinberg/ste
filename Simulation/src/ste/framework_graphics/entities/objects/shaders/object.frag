
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

#include "material.glsl"
#include "gbuffer.glsl"

in vec2 frag_texcoords;
in vec3 frag_position;
in vec3 frag_normal;
in vec3 frag_tangent;
flat in int matIdx;

uniform float height_map_scale = 1.f;

void main() {
	vec2 uv = frag_texcoords;
	material_descriptor md = mat_descriptor[matIdx];

	float alpha = md.alphamap.tex_handler>0 ? texture(sampler2D(md.alphamap.tex_handler), uv).x : 1.f;
	if (alpha == .0f) {
		discard;
		return;
	}

	vec3 P = frag_position;
	vec3 n = normalize(frag_normal);
	vec3 t = normalize(frag_tangent);

	normal_map(md, height_map_scale, uv, n, t, P);

	float specular = md.specular.tex_handler>0 ? texture(sampler2D(md.specular.tex_handler), uv).x : 1.f;
	vec4 diffuse = md.diffuse.tex_handler>0 ? texture(sampler2D(md.diffuse.tex_handler), uv) : vec4(1.f);
	diffuse.a *= alpha;

	gbuffer_store(P, diffuse, specular, n, t, matIdx, ivec2(gl_FragCoord.xy));
	gl_FragDepth = alpha == 1.f ? gl_FragCoord.z : 1.f;
}
