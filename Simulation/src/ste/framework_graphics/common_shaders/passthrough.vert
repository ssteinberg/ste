
#type vert
#version 440

layout(location = 0) in vec3 vert;
layout(location = 1) in vec2 tc;

out vec4 gl_Position;
out vs_out {
	vec2 tex_coords;
} vout;

void main() {
	vout.tex_coords = tc;
	gl_Position = vec4(vert, 1);
}
