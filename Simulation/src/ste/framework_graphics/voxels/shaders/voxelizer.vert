
#type vert
#version 450
#extension GL_ARB_shader_draw_parameters : require

#include "girenderer_matrix_buffer.glsl"
#include "mesh_descriptor.glsl"

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 tex_coords;

out vs_out {
	vec3 N;
	vec2 st;
	flat int matIdx;
} vout;

uniform vec3 translation;

layout(std430, binding = 1) restrict readonly buffer mesh_data {
	mesh_descriptor mesh_descriptor_buffer[];
};
layout(std430, binding = 12) restrict readonly buffer id_to_drawid_data {
	uint id_to_drawid[];
};

void main() {
	uint draw_id = id_to_drawid[gl_DrawIDARB];
	mesh_descriptor md = mesh_descriptor_buffer[draw_id];

	mat4 model = md.model;
	mat4 trans_inverse_model = view_matrix_buffer.transpose_inverse_view_matrix * md.transpose_inverse_model;

	vec4 P = model * vec4(vert, 1);
	P.xyz += translation;

	vout.N = (trans_inverse_model * vec4(normal, 1)).xyz;
	vout.st = tex_coords;
	vout.matIdx = md.matIdx;

    gl_Position = P;
}

