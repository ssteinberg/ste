
#type frag
#version 440

out vec3 gl_FragColor;

in vec4 position;

uniform float far;

void main() {
	gl_FragDepth = length(position.xyz) / far;
}
