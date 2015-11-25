
#type geometry
#version 450
#extension GL_ARB_bindless_texture : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "material.glsl"
#include "voxels.glsl"

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vs_out {
	vec3 N, T;
	vec2 st;
	flat int matIdx;
} vin[];

out geo_out {
	vec3 P, N, T;
	vec2 st;
	flat int matIdx;
	flat vec2 aabb_min;
	flat vec2 aabb_max;
} vout;

void main() {
	vec3 pos0 = gl_in[0].gl_Position.xyz;
	vec3 pos1 = gl_in[1].gl_Position.xyz;
	vec3 pos2 = gl_in[2].gl_Position.xyz;

	vec3 center = (pos0 + pos1 + pos2) / 3.f;

	float voxel = voxel_size(min(min(pos0, pos1), pos2));
	vec3 v0 = pos0 - center;
	vec3 v1 = pos1 - center;
	vec3 v2 = pos2 - center;
	v0 /= voxel;
	v1 /= voxel;
	v2 /= voxel;
	
	vec3 edges_cross = cross(v2-v0, v1-v0);
	float area = .5f * dot(edges_cross, edges_cross);
	if (area < voxel) {
		// Voxelize in geometry shader
		// TODO
	}

	v0 = sign(v0) * ceil(sign(v0) * v0);
	v1 = sign(v1) * ceil(sign(v1) * v1);
	v2 = sign(v2) * ceil(sign(v2) * v2);
	
	vec3 T = normalize(pos1 - pos0);
	vec3 N = normalize(cross(T, pos2 - pos0));
	vec3 B = cross(T, N);
	mat3 TBN = transpose(mat3(T, B, N));
	
	int voxels_texture_size = imageSize(voxel_levels[0]).x;
	vec3 p0 = 2.f * TBN * v0 / voxels_texture_size;
	vec3 p1 = 2.f * TBN * v1 / voxels_texture_size;
	vec3 p2 = 2.f * TBN * v2 / voxels_texture_size;
	
	vout.matIdx = vin[0].matIdx;
	vout.aabb_min = min(min(p0.xy, p1.xy), p2.xy);
	vout.aabb_max = max(max(p0.xy, p1.xy), p2.xy);
	
	vout.st = vin[0].st;
	vout.P = pos0;
	vout.N = vin[0].N;
	vout.T = vin[0].T;
    gl_Position = vec4(p0.xy, 0, 1);
    EmitVertex();
	
	vout.st = vin[1].st;
	vout.P = pos1;
	vout.N = vin[1].N;
	vout.T = vin[1].T;
    gl_Position = vec4(p1.xy, 0, 1);
    EmitVertex();
	
	vout.st = vin[2].st;
	vout.P = pos2;
	vout.N = vin[2].N;
	vout.T = vin[2].T;
    gl_Position = vec4(p2.xy, 0, 1);
    EmitVertex();

    EndPrimitive();
}
