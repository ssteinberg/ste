
#type vert
#version 450
#extension GL_ARB_shader_draw_parameters : enable

#include "mesh_descriptor.glsl"

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 normal;
layout(location = 3) in vec2 tex_coords;

layout(std430, binding = 1) restrict readonly buffer mesh_data {
	mesh_descriptor mesh_descriptor_buffer[];
};

out vs_out {
	vec3 normal;
	vec2 uv;
	flat int matIdx;
} vout;

void main() {
	uint draw_id = gl_BaseInstanceARB;
	mesh_descriptor md = mesh_descriptor_buffer[draw_id];

	mat4 view_model = md.model;
	mat4 trans_inverse_view_model = md.transpose_inverse_model;

	gl_Position = view_model * vec4(vert, 1);
	vout.normal = (trans_inverse_view_model * vec4(normal, 1)).xyz;
	vout.uv = tex_coords;
	vout.matIdx = md.matIdx;
}
