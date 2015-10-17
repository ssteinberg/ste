
#type vert
#version 450

layout(location = 0) in vec2 vert;
layout(location = 1) in float coc;
layout(location = 2) in vec4 color;

uniform mat4 proj;

out vec4 gl_Position;

out vs_out {
	vec4 color;
	float coc;
} vout;

void main() {
	vout.coc = coc;
	vout.color = color;
    gl_Position = proj * vec4(vert, 0, 1);
}
