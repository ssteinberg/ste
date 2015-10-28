
#type vert
#version 440

layout(location = 1) in vec3 vert;

out vec4 gl_Position;
out vec3 l_pos;

uniform mat4 view;

void main() {
	vec4 v = view * vec4(-1000, 150.251404, -127.7326050, 1);
	l_pos = v.xyz;

	gl_Position = vec4(vert, 1);
}
