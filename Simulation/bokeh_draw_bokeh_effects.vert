
#type vert
#version 450

layout(location = 0) in vec4 pos_size;
layout(location = 1) in vec4 col;

out vs_out {
	vec4 color;
	float coc;
} vout;

void main() {
	vout.coc = pos_size.z;
	vout.color = col;
	gl_Position = vec4(pos_size.xy, 0, 1);
}
