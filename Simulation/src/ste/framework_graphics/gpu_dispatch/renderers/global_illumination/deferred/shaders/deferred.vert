
#type vert
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

const int light_buffers_first = 2;

#include "light.glsl"

layout(location = 0) in vec3 vert;
layout(location = 3) in vec2 tc;

out vec4 gl_Position;
out vec2 tex_coords;

uniform mat4 view_matrix;

void main() {
	tex_coords = tc;
	gl_Position = vec4(vert, 1);

	light_transform(view_matrix, mat3(view_matrix));
}
