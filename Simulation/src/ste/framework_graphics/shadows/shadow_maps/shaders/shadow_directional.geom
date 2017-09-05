
#type geometry
#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

#include <light.glsl>
#include <light_cascades.glsl>

#include <project.glsl>

#include <shadow_drawid_to_lightid_ttl.glsl>

in vs_out {
	vec3 position;
	flat int instanceIdx;
	flat uint drawIdx;
} vin[];

layout(std430, set=0, binding=0) restrict readonly buffer d_drawid_to_lightid_ttl_data {
	d_drawid_to_lightid_ttl ttl[];
};

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
	vec3 u = vin[2].position - vin[1].position;
	vec3 v = vin[0].position - vin[1].position;
	vec3 n = cross(u,v);

	if (dot(n,-l) <= 0)
		return;
		
	// Read cascade descriptor and per cascade transformation matrix, transform vertices and output
	uint cascade_idx = light_get_cascade_descriptor_idx(ld);
	
	for (int cascade = 0; cascade < directional_light_cascades; ++cascade) {
		float z_far = light_cascades[cascade_idx].cascades[cascade].proj_far_clip;
		mat3x4 M = light_cascades[cascade_idx].cascades[cascade].M;

		vec3 vertices[3];
		for (int j = 0; j < 3; ++j)
			vertices[j] = transform(vec4(vin[j].position, 1), 
									M);

		process(cascade, cascade_idx, vertices, z_far);
	}
}
