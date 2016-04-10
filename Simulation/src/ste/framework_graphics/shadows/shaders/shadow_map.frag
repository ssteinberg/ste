
#type frag
#version 440

out vec3 gl_FragColor;

in vec4 position;

void main() {
	gl_FragDepth = length(position.xyz) / 3000.f;
}
