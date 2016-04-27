
#type frag
#version 450
#extension GL_NV_gpu_shader5 : require

#include "material.glsl"

layout(std430, binding = 0) restrict readonly buffer material_data {
	material_descriptor mat_descriptor[];
};

in vs_out {
	vec2 uv;
	flat int matIdx;
} vin;

void main() {
	material_descriptor md = mat_descriptor[vin.matIdx];

	vec4 diffuse = md.diffuse.tex_handler>0 ? texture(sampler2D(md.diffuse.tex_handler), vin.uv) : vec4(1.f);
	float alpha = md.alphamap.tex_handler>0 ? texture(sampler2D(md.alphamap.tex_handler), vin.uv).x : 1.f;
	if (diffuse.a * alpha < 1.f)
		discard;

	gl_FragDepth = gl_FragCoord.z + .00001f;
}
