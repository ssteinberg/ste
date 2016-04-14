
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

#include "material.glsl"

in frag_in {
	vec4 position;
	vec2 uv;
	flat int matIdx;
} vin;

uniform float far;

void main() {
	material_descriptor md = mat_descriptor[vin.matIdx];

	if (md.alphamap.tex_handler>0 && texture(sampler2D(md.alphamap.tex_handler), vin.uv).x<.5f) {
		discard;
		return;
	}

	// In case we want to consider semi-transparent objects
	// vec3 diffuse = md.diffuse.tex_handler>0 ? texture(sampler2D(md.diffuse.tex_handler), uv).rgb : vec3(1.f);

	gl_FragDepth = length(vin.position.xyz) / far;
}
