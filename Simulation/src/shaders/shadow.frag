
#type frag
#version 440

in vec4 position;

uniform vec3 light_pos;
uniform float far;

void main() {
	gl_FragDepth = length(position.xyz - light_pos) / far;
}
