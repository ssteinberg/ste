
#type geometry
#version 450

#extension GL_ARB_bindless_texture : enable

layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

#include "light.glsl"
#include "light_cascades.glsl"

#include "project.glsl"

#include "shadow.glsl"
#include "shadow_drawid_to_lightid_ttl.glsl"

in vs_out {
	flat int instanceIdx;
	flat uint drawIdx;
} vin[];

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};

layout(shared, binding = 6) restrict readonly buffer directional_lights_cascades_data {
	light_cascade_descriptor directional_lights_cascades[];
};

layout(shared, binding = 8) restrict readonly buffer d_drawid_to_lightid_ttl_data {
	d_drawid_to_lightid_ttl ttl[];
};

uniform float cascades_depths[directional_light_cascades];

vec3 transform(vec4 v, mat3x4 M) {
	return v * M;
}

void process(int cascade, uint cascade_idx, vec3 vertices[3], float f) {
	float n = cascade_projection_near_clip;

	// Cull triangles outside the NDC
	if ((vertices[0].x >  1 &&
		 vertices[1].x >  1 &&
		 vertices[2].x >  1) ||
		(vertices[0].x < -1 &&
		 vertices[1].x < -1 &&
		 vertices[2].x < -1) ||
		(vertices[0].y >  1 &&
		 vertices[1].y >  1 &&
		 vertices[2].y >  1) ||
		(vertices[0].y < -1 &&
		 vertices[1].y < -1 &&
		 vertices[2].y < -1) ||
		(vertices[0].z < -f &&
		 vertices[1].z < -f &&
		 vertices[2].z < -f))
		return;

	for (int j = 0; j < 3; ++j) {
		gl_Layer = cascade + int(cascade_idx) * directional_light_cascades;

		// Clamp z values behind the near-clip plane to the near-clip distance, this geometry participates in directional shadows as well.
		float z = min(-n - 1e-8f, vertices[j].z);
		
		// Orthographic projection
		gl_Position.xy = vertices[j].xy;
		gl_Position.z = project_depth_linear(z, n, f);
		gl_Position.w = 1.f;

		EmitVertex();
	}

	EndPrimitive();
}

void main() {
	int sproj_instance_id = vin[0].instanceIdx;
	uint draw_id = vin[0].drawIdx;
	uint light_idx = translate_drawid_to_light_idx(ttl[draw_id].entries[sproj_instance_id]);

	light_descriptor ld = light_buffer[light_idx];

	// Calculate normal and cull back faces
	vec3 l = ld.transformed_position;
	vec3 u = gl_in[2].gl_Position.xyz - gl_in[1].gl_Position.xyz;
	vec3 v = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;
	vec3 n = cross(u,v);

	if (dot(n,-l) <= 0)
		return;
		
	// Read cascade descriptor and per cascade build the transformation matrix, transform vertices and output
	uint cascade_idx = light_get_cascade_descriptor_idx(ld);
	light_cascade_descriptor cascade_descriptor = directional_lights_cascades[cascade_idx];
	
	for (int cascade = 0; cascade < directional_light_cascades; ++cascade) {
		float z_far;
		mat3x4 M = light_cascade_projection(cascade_descriptor, cascade, l, cascades_depths, z_far);

		vec3 vertices[3];
		for (int j = 0; j < 3; ++j)
			vertices[j] = transform(gl_in[j].gl_Position, M);

		process(cascade, cascade_idx, vertices, z_far);
	}
}
