
#type vert
#version 440

layout(location = 0) in vec3 vert;

uniform mat4 model_mat;

void main() {
	gl_Position = model_mat * vec4(vert, 1.0);
}
