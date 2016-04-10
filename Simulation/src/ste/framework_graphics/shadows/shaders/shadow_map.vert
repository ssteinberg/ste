
#type vert
#version 450
#extension GL_ARB_shader_draw_parameters : enable

#include "mesh.glsl"

layout(location = 0) in vec3 vert;
layout(location = 3) in vec2 tex_coords;

out vec4 gl_Position;

layout(std430, binding = 1) buffer mesh_data {
	mesh_descriptor mesh_descriptor_buffer[];
};

uniform vec4 light_pos;

void main() {
	mesh_descriptor md = mesh_descriptor_buffer[gl_DrawIDARB];
	mat4 view_model = md.model;

	gl_Position = view_model * vec4(vert, 1) - light_pos;
}
