
#version 440

layout(location = 1) in vec3 vert;

out vec4 gl_Position;

void main() {
	gl_Position = vec4(vert, 1);
}
