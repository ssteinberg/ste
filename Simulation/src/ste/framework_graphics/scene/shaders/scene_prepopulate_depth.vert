
#type vert
#version 450
#extension GL_ARB_shader_draw_parameters : enable

#include "girenderer_matrix_buffer.glsl"
#include "mesh_descriptor.glsl"

layout(location = 0) in vec3 vert;
layout(location = 3) in vec2 tex_coords;

layout(std430, binding = 1) restrict readonly buffer mesh_data {
	mesh_descriptor mesh_descriptor_buffer[];
};

out vs_out {
	vec2 uv;
	flat int matIdx;
} vout;

void main() {
	uint draw_id = gl_BaseInstanceARB;
	mesh_descriptor md = mesh_descriptor_buffer[draw_id];

	gl_Position = view_matrix_buffer.projection_view_matrix * (md.model * vec4(vert, 1));
	vout.uv = tex_coords;
	vout.matIdx = md.matIdx;
}
