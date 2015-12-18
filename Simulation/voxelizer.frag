
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
	flat vec3 ortho_projection_mask;
} fragment;

void main() {
	if (gl_FragCoord.x > fragment.max_aabb.x || gl_FragCoord.y > fragment.max_aabb.y) {
		discard;
		return;
	}

	int matIdx = fragment.matIdx;
	vec2 uv = fragment.st;
	vec3 P = fragment.P;
	vec3 N = fragment.N;
	material_descriptor md = mat_descriptor[matIdx];

	vec2 dUVdx = dFdx(uv) * 4;
	vec2 dUVdy = dFdy(uv) * 4;
	
	uint level = voxel_level(P);
	float size = voxel_size(level);

	P /= size;
	vec3 C = round(P);
	
	voxelize(md, ivec3(C), uv, N, level, size, 1.f, dUVdx, dUVdy);

	
	vec3 dPdx = dFdx(P) * .5f;
	vec3 dPdy = dFdy(P) * .5f;
	
	vec3 P00 = round(P - dPdx - dPdy) * fragment.ortho_projection_mask;
	vec3 P10 = round(P + dPdx - dPdy) * fragment.ortho_projection_mask;
	vec3 P01 = round(P - dPdx + dPdy) * fragment.ortho_projection_mask;
	vec3 P11 = round(P + dPdx + dPdy) * fragment.ortho_projection_mask;
	
	vec3 Cplus1 = C * fragment.ortho_projection_mask + fragment.ortho_projection_mask;
	vec3 Cminus1 = C * fragment.ortho_projection_mask - fragment.ortho_projection_mask;

	if (P00 == Cplus1 || P10 == Cplus1 || P01 == Cplus1 || P11 == Cplus1)
		voxelize(md, ivec3(C + fragment.ortho_projection_mask), uv, N, level, size, 1.f, dUVdx, dUVdy);

	if (P00 == Cminus1 || P10 == Cminus1 || P01 == Cminus1 || P11 == Cminus1)
		voxelize(md, ivec3(C - fragment.ortho_projection_mask), uv, N, level, size, 1.f, dUVdx, dUVdy);
}
