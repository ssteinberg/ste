
#type vert
#version 440

out vec2 uv;

void main() {
	uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(mix(vec2(-1.f), vec2(1.f), uv), .0f, 1.f);
}
