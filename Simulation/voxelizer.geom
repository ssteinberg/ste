
#type geometry
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_shader_atomic_fp16_vector : require
#extension GL_NV_gpu_shader5 : require

#include "material.glsl"
#include "voxels.glsl"
 
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vs_out {
	vec3 N;
	vec2 st;
	flat int matIdx;
} vin[];

out geo_out {
	vec3 P, N;
	vec2 st;
	flat int matIdx;
	flat vec2 max_aabb;
} vout;

vec2 bounding_triangle_vertex(vec3 U, vec3 V, vec3 W) {
	vec3 prev = vec3(U.xy, 1);
	vec3 v = vec3(V.xy, 1);
	vec3 next = vec3(W.xy, 1);

	vec3 a = v - prev;
	vec3 b = next - v;

	vec3 p0 = cross(a, prev);
	vec3 p1 = cross(b, v);

	p0.z -= dot(vec2(.5f), abs(p0.xy));
	p1.z -= dot(vec2(.5f), abs(p1.xy));

	vec3 t = cross(p0, p1);
	return t.xy / t.z;
}

void main() {
	int matIdx = vin[0].matIdx;

	vec3 U = gl_in[0].gl_Position.xyz;
	vec3 V = gl_in[1].gl_Position.xyz;
	vec3 W = gl_in[2].gl_Position.xyz;
	
	vec3 T = V - U;
	vec3 unnormedN = cross(T, W - U);
	float len_N = length(unnormedN);
	vec3 N = unnormedN / len_N;
	
	vec3 min_world_aabb = min(U, min(V, W));
	vec3 max_world_aabb = max(U, max(V, W));
	vec3 aabb_signs = sign(min_world_aabb) * sign(max_world_aabb) * 2 - vec3(1);
	vec3 aabb_distances = min(abs(min_world_aabb), abs(max_world_aabb));
	vec3 ds = abs(aabb_signs) * aabb_distances;

	float d = max(min(min(ds.x, ds.y), ds.z), dot(-U, N));
	float voxel = voxel_size(voxel_level(d));
	
	vec3 pos0 = U / voxel;
	vec3 pos1 = V / voxel;
	vec3 pos2 = W / voxel;
	
	float area = .5f * len_N / (voxel * voxel);
	if (area < .66f) {
		// Voxelize in geometry shader. Fast path.
		material_descriptor md = mat_descriptor[matIdx];
		vec2 uv = vec2(.5f);
		voxelize(md, 
				 U, 
				 uv, 
				 (vin[0].N + vin[1].N + vin[2].N) / 3.f, 
				 area,
				 vec2(1),
				 vec2(1));

		return;
	}

	T = normalize(T);
	vec3 B = cross(N, T);

	mat3 TBN = mat3(T, B, N);
	mat3 invTBN = transpose(TBN);
	
	float voxels_texture_size = .4f * float(textureSize(voxel_space_radiance, 0).x);
	vec3 p0 = invTBN * pos0;
	vec3 p1 = invTBN * pos1;
	vec3 p2 = invTBN * pos2;
	vec2 minv = min(min(p0.xy, p1.xy), p2.xy);

	vout.max_aabb = max(max(p0.xy, p1.xy), p2.xy) - minv;

	p0.xy = bounding_triangle_vertex(p2, p0, p1);
	p1.xy = bounding_triangle_vertex(p0, p1, p2);
	p2.xy = bounding_triangle_vertex(p1, p2, p0);
	
	U = TBN * (p0 * voxel);
	V = TBN * (p1 * voxel);
	W = TBN * (p2 * voxel);

	vec2 v0 = p0.xy - minv;
	vec2 v1 = p1.xy - minv;
	vec2 v2 = p2.xy - minv;

	v0 /= voxels_texture_size;
	v1 /= voxels_texture_size;
	v2 /= voxels_texture_size;
	
	vout.matIdx = matIdx;
	
	vout.st = vin[0].st;
	vout.P = U;
	vout.N = vin[0].N;
    gl_Position = vec4(v0 - vec2(1, 1), 0, 1);
    EmitVertex();
	
	vout.st = vin[1].st;
	vout.P = V;
	vout.N = vin[1].N;
    gl_Position = vec4(v1 - vec2(1, 1), 0, 1);
    EmitVertex();
	
	vout.st = vin[2].st;
	vout.P = W;
	vout.N = vin[2].N;
    gl_Position = vec4(v2 - vec2(1, 1), 0, 1);
    EmitVertex();

    EndPrimitive();
}
