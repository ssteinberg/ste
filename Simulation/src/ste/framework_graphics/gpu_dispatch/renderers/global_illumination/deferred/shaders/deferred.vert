
#type vert
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

const int light_buffers_first = 1;

#include "light.glsl"

layout(location = 0) in vec3 vert;

out vec4 gl_Position;

uniform mat4 view_matrix;

void main() {
	gl_Position = vec4(vert, 1);
	
	light_transform(view_matrix, mat3(view_matrix));
}
