
#type geometry
#version 450

#include <common.glsl>
#include <material.glsl>
#include <voxels.glsl>

const float voxelizer_fb_extent = 16384.f;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout(location = 0) in vs_out {
	vec3 N;
	vec3 T;
	vec2 st;
	flat int material_id;
} vin[];

layout(location = 0) out geo_out {
	vec3 P, N;
	float transformed_triangle_z;
	vec2 st;
	flat int material_id;
	flat vec2 max_aabb;
	flat vec3 ortho_projection_mask;
} vout;

/**
*	@brief	Offsets triangle's vertices for conservative rasterization
*			Expects front-facing CCW triangles.
*/
vec2 bounding_triangle_vertex(vec2 prev, vec2 v, vec2 next) {
	vec3 U = vec3(prev, 1);
	vec3 V = vec3(v, 1);
	vec3 W = vec3(next, 1);

	vec3 a = V - U;
	vec3 b = W - V;

	vec3 p0 = cross(a, U);
	vec3 p1 = cross(b, V);

	p0.z -= dot(vec2(.5f), abs(p0.xy));
	p1.z -= dot(vec2(.5f), abs(p1.xy));

	vec3 t = cross(p0, p1);
	return t.xy / t.z;
}

void main() {
	// Read vertices and compute normal
	vec3 U = gl_in[0].gl_Position.xyz;
	vec3 V = gl_in[1].gl_Position.xyz;
	vec3 W = gl_in[2].gl_Position.xyz;

	vec3 N = normalize(cross(V - U, W - U));

	// Create transform matrix T
	// invT rotates the triangle s.t. it "faces the screen".
	mat3 T;
	vec3 absN = abs(N);
	vec3 signN = sign(N);
	if (absN.x >= absN.y && absN.x >= absN.z) {
		T = mat3(0,0,-signN.x,
				 0,1,0,
				 signN.x,0,0);
		vout.ortho_projection_mask = vec3(1,0,0);
	}
	else if (absN.y >= absN.x && absN.y >= absN.z) {
		T = mat3(1,0,0,
				 0,0,-signN.y,
				 0,signN.y,0);
		vout.ortho_projection_mask = vec3(0,1,0);
	}
	else {
		T = mat3(signN.z,0,0,
				 0,1,0,
				 0,0,signN.z);
		vout.ortho_projection_mask = vec3(0,0,1);
	}
	mat3 invT = transpose(T);

	// Compute scale factor with which to scale the vertices by voxel grid resolution
	float s = voxel_grid_resolution;

	// Compute scaled, transformed vertices
	vec3 p0 = 1.f / s * (invT * U);
	vec3 p1 = 1.f / s * (invT * V);
	vec3 p2 = 1.f / s * (invT * W);

	// Compute AABB
	vec2 minv = min(p0.xy, p1.xy, p2.xy);
	vout.max_aabb = max(p0.xy, p1.xy, p2.xy) - minv + vec2(1);

	N = invT * N;
	float d = dot(p0, N);

	// Offset triangle for conservative rasterization
	p0.xy = bounding_triangle_vertex(p2.xy, p0.xy, p1.xy);
	p1.xy = bounding_triangle_vertex(p0.xy, p1.xy, p2.xy);
	p2.xy = bounding_triangle_vertex(p1.xy, p2.xy, p0.xy);
	p0.z = (d - dot(p0.xy, N.xy)) / N.z;
	p1.z = (d - dot(p1.xy, N.xy)) / N.z;
	p2.z = (d - dot(p2.xy, N.xy)) / N.z;

	// Compute actual world coordinates of offseted, unscaled, untransformed triangle
	U = s * (T * p0);
	V = s * (T * p1);
	W = s * (T * p2);

	// Normalize to framebuffer extent
	vec2 v0 = (p0.xy - minv) / voxelizer_fb_extent - vec2(1, 1);
	vec2 v1 = (p1.xy - minv) / voxelizer_fb_extent - vec2(1, 1);
	vec2 v2 = (p2.xy - minv) / voxelizer_fb_extent - vec2(1, 1);

	// Emit vertices
	vout.material_id = vin[0].material_id;
	vout.st = vin[0].st;
	vout.P = U;
	vout.N = vin[0].N;
	vout.transformed_triangle_z = p0.z;
	gl_Position = vec4(v0, 0, 1);
	EmitVertex();

	vout.st = vin[1].st;
	vout.P = V;
	vout.N = vin[1].N;
	vout.transformed_triangle_z = p1.z;
	gl_Position = vec4(v1, 0, 1);
	EmitVertex();

	vout.st = vin[2].st;
	vout.P = W;
	vout.N = vin[2].N;
	vout.transformed_triangle_z = p2.z;
	gl_Position = vec4(v2, 0, 1);
	EmitVertex();

	EndPrimitive();
}
