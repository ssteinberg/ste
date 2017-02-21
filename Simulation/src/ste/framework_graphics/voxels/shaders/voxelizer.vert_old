
#type vert
#version 450
#extension GL_ARB_shader_draw_parameters : require

#include <girenderer_transform_buffer.glsl>
#include <mesh_descriptor.glsl>

layout(location = 0) in vec4 tangent_frame_quat;
layout(location = 1) in vec3 vert;
layout(location = 2) in vec2 tex_coords;

out vs_out {
	vec3 N;
	vec2 st;
	flat int matIdx;
} vout;

uniform vec3 translation;

layout(std430, binding = 1) restrict readonly buffer mesh_data {
	mesh_descriptor mesh_descriptor_buffer[];
};

void main() {
	uint draw_id = gl_BaseInstanceARB;
	mesh_descriptor md = mesh_descriptor_buffer[draw_id];

	mat4 model = md.model;
	mat4 trans_inverse_model = view_transform_buffer.transpose_inverse_view_matrix * md.transpose_inverse_model;

	vec4 P = model * vec4(vert, 1);
	P.xyz += translation;

	// vout.N = (trans_inverse_model * vec4(normal, 1)).xyz;
	vout.st = tex_coords;
	vout.matIdx = md.matIdx;

    gl_Position = P;
}

