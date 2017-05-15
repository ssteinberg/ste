
#type vert
#version 450

out vec2 uv;

void main() {
	uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	vec2 p = vec2(1,-1);
	gl_Position = vec4(mix(p, -p, uv), .0f, 1.f);
}
