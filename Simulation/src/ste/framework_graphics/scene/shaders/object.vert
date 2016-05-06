
#type vert
#version 450
#extension GL_ARB_shader_draw_parameters : require

#include "mesh_descriptor.glsl"
#include "girenderer_matrix_buffer.glsl"

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 tex_coords;

out vec4 gl_Position;
out v {
	vec3 frag_position;
	vec2 frag_texcoords;
	vec3 frag_normal;
	vec3 frag_tangent;
	flat int matIdx;
} vout;

layout(std430, binding = 14) restrict readonly buffer mesh_data {
	mesh_descriptor mesh_descriptor_buffer[];
};

void main() {
	uint draw_id = gl_BaseInstanceARB;
	mesh_descriptor md = mesh_descriptor_buffer[draw_id];

	mat4 trans_inverse_view_model = view_matrix_buffer.transpose_inverse_view_matrix * md.transpose_inverse_model;

	vec4 wpos = md.model * vec4(vert, 1);

	vout.frag_position = (view_matrix_buffer.view_matrix * wpos).xyz;
	vout.frag_texcoords = tex_coords;
	vout.frag_normal = (trans_inverse_view_model * vec4(normal, 1)).xyz;
	vout.frag_tangent = (trans_inverse_view_model * vec4(tangent, 1)).xyz;
	vout.matIdx = md.matIdx;

	gl_Position = view_matrix_buffer.projection_view_matrix * wpos;
}
