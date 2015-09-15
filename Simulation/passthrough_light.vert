
#version 440

layout(location = 1) in vec3 vert;

out vec4 gl_Position;
out vec3 l_pos;

uniform mat4 view;

void main() {
	l_pos = (view * vec4(-70,150,-45,1)).xyz;
	gl_Position = vec4(vert, 1);
}
