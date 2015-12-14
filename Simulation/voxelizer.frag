
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_shader_atomic_fp16_vector : require
#extension GL_NV_gpu_shader5 : require

#include "material.glsl"
#include "voxels.glsl"

in geo_out {
	vec3 P, N;
	vec2 st;
	flat int matIdx;
	flat vec2 max_aabb;
	flat float raster_size;
} fragment;

void main() {
	float voxels_texture_size = fragment.raster_size;
	vec2 p = 2.f * gl_FragCoord.xy / voxels_texture_size;
	if (p.x > fragment.max_aabb.x || p.y > fragment.max_aabb.y) {
		discard;
		return;
	}

	int matIdx = fragment.matIdx;
	vec2 uv = fragment.st;
	material_descriptor md = mat_descriptor[matIdx];

	voxelize(md, fragment.P, uv, fragment.N, 1.f, dFdx(uv) * 2, dFdy(uv) * 2);
}
