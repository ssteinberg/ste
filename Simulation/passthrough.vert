
#version 440

layout(location = 0) in vec2 tc;
layout(location = 1) in vec3 vert;

out vec4 gl_Position;
out vec2 tex_coords;

void main() {
	tex_coords = tc;
	gl_Position = vec4(vert, 1);
}
