
#type frag
#version 450

out vec4 gl_FragColor;

in geo_out {
	vec4 color;
	vec2 st;
} vin;

layout(binding = 0) uniform sampler2D tex;

void main() {
    gl_FragColor = vec4(1, 0, 0, 1);
}
