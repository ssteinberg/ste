
#type frag
#version 450
#extension GL_ARB_bindless_texture : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "material.glsl"
#include "voxels.glsl"

in geo_out {
	vec3 P, N, T;
	vec2 st;
	flat int matIdx;
	flat vec2 aabb_min;
	flat vec2 aabb_max;
} fragment;

void main() {
	int matIdx = fragment.matIdx;
	vec2 uv = fragment.st;

	material_descriptor md = mat_descriptor[matIdx];
	vec3 diffuse = md.diffuse.tex_handler>0 ? texture(sampler2D(md.diffuse.tex_handler), uv).rgb : vec3(1.f);

	voxelize(fragment.P, vec4(diffuse, 1));
}
