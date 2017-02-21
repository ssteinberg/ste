
#type geometry
#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

#include <light.glsl>
#include <shadow_drawid_to_lightid_ttl.glsl>

#include <project.glsl>

in vs_out {
	flat int instanceIdx;
	flat uint drawIdx;
} vin[];

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};

layout(std430, binding = 8) restrict readonly buffer drawid_to_lightid_ttl_data {
	drawid_to_lightid_ttl ttl[];
};

const vec2 t = vec2(1,-1);

vec4 transform(int face, vec3 v, float n, float f) {
	vec4 u;

	// Transformation per face
	if (face == 0)		u.xyz =-v.zyx;
	else if (face == 1) u.xyz = v.zyx * t.xyx;
	else if (face == 2) u.xyz = v.xzy * t.xxy;
	else if (face == 3) u.xyz = v.xzy * t.xyx;
	else if (face == 4) u.xyz = v.xyz * t.xyy;
	else				u.xyz = v.xyz * t.yyx;
	
	// Inverse projection with near and far clips
	u.w = -u.z;
	u.z = n;//project_depth_linear(u.z, n, f) * u.w;//(u.z + f) * n / (f-n);
	return u;
}

void process(int face, uint l, vec3 vertices[3], float shadow_near, float f) {
	// Transform to cube face and project
	vec4 transformed_vertices[3];
	for (int j = 0; j < 3; ++j)
		transformed_vertices[j] = transform(face, vertices[j], shadow_near, f);

	// Cull triangles outside the NDC
	if ((transformed_vertices[0].x >  transformed_vertices[0].w && 
		 transformed_vertices[1].x >  transformed_vertices[1].w && 
		 transformed_vertices[2].x >  transformed_vertices[2].w) || 
		(transformed_vertices[0].x < -transformed_vertices[0].w && 
		 transformed_vertices[1].x < -transformed_vertices[1].w && 
		 transformed_vertices[2].x < -transformed_vertices[2].w) || 
		(transformed_vertices[0].y >  transformed_vertices[0].w && 
		 transformed_vertices[1].y >  transformed_vertices[1].w && 
		 transformed_vertices[2].y >  transformed_vertices[2].w) || 
		(transformed_vertices[0].y < -transformed_vertices[0].w && 
		 transformed_vertices[1].y < -transformed_vertices[1].w && 
		 transformed_vertices[2].y < -transformed_vertices[2].w) || 
		(transformed_vertices[0].z < -transformed_vertices[0].w && 
		 transformed_vertices[1].z < -transformed_vertices[1].w && 
		 transformed_vertices[2].z < -transformed_vertices[2].w) || 
		(transformed_vertices[0].w >  f && 
		 transformed_vertices[1].w >  f && 
		 transformed_vertices[2].w >  f))
	return;

	for (int j = 0; j < 3; ++j) {
		gl_Layer = face + int(l) * 6;
		gl_Position = transformed_vertices[j];
		EmitVertex();
	}

	EndPrimitive();
}

void main() {
	int sproj_instance_id = vin[0].instanceIdx;
	uint draw_id = vin[0].drawIdx;

	drawid_to_lightid_ttl_entry ttl_entry = ttl[draw_id].entries[sproj_instance_id];
	uint ll_idx = translate_drawid_to_ll_idx(ttl_entry);
	uint light_idx = translate_drawid_to_light_idx(ttl_entry);

	light_descriptor ld = light_buffer[light_idx];

	float near_clip = ld.radius * 2.f;
	vec3 light_pos = ld.position;
	float light_range = light_effective_range(ld);
	float light_range2 = light_range * light_range;

	// Back face culling
	vec3 u = gl_in[2].gl_Position.xyz - gl_in[1].gl_Position.xyz;
	vec3 v = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;
	vec3 N = cross(u,v);
	vec3 V = light_pos.xyz - gl_in[1].gl_Position.xyz;
	
	if (dot(N,V) <= 0)
		return;

	vec3 vertices[3];

	// Range culling
	int out_of_range = 0;
	for (int j = 0; j < 3; ++j) {
		vec3 P = gl_in[j].gl_Position.xyz - light_pos;
		if (dot(P,P) >= light_range2)
			++out_of_range;

		vertices[j] = P;
	}
	if (out_of_range == 3)
		return;

	// Transform and output
	for (int face = 0; face < 6; ++face) 
		process(face, ll_idx, vertices, near_clip, light_range);
}
