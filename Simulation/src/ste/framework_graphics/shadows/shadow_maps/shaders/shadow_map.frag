
#type frag
#version 450
#extension GL_NV_gpu_shader5 : require

#include "material.glsl"

in frag_in {
	vec2 uv;
	flat int matIdx;
} vin;

void main() {
	// material_descriptor md = mat_descriptor[vin.matIdx];

	// float alpha = md.alphamap.tex_handler>0 ? texture(sampler2D(md.alphamap.tex_handler), vin.uv).x : 1.f;
	// if (alpha < 1.f) {
	// 	discard;
	// 	return;
	// }
}
