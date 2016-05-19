
#type frag
#version 450
#extension GL_NV_gpu_shader5 : require

#include "material.glsl"

layout(std430, binding = 13) restrict readonly buffer material_data {
	material_descriptor mat_descriptor[];
};

in vs_out {
	vec2 uv;
	flat int matIdx;
} vin;

void main() {
	material_descriptor md = mat_descriptor[vin.matIdx];

	float alpha = md.alphamap.tex_handler>0 ? texture(sampler2D(md.alphamap.tex_handler), vin.uv).x : 1.f;
	if (alpha < .5f)
		discard;
}
