
#type vert
#version 450
#extension GL_ARB_shader_draw_parameters : enable

#include "girenderer_transform_buffer.glsl"
#include "mesh_descriptor.glsl"

layout(location = 1) in vec3 vert;
layout(location = 2) in vec2 tex_coords;

layout(std430, binding = 14) restrict readonly buffer mesh_data {
	mesh_descriptor mesh_descriptor_buffer[];
};

out vs_out {
	vec2 uv;
	flat int matIdx;
} vout;

void main() {
	uint draw_id = gl_BaseInstanceARB;
	mesh_descriptor md = mesh_descriptor_buffer[draw_id];

	vec3 wpos = (md.model * vec4(vert, 1)).xyz;
	vec3 spos = dquat_mul_vec(view_transform_buffer.view_transform, wpos.xyz);

	vout.uv = tex_coords;
	vout.matIdx = md.matIdx;

	gl_Position = project(spos);
}
