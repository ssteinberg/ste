
#type frag
#version 450

out vec4 gl_FragColor;

layout(binding = 0) uniform sampler2D tex;

uniform vec2 insize;
uniform vec2 outsize;
uniform vec2 filter_dir;
uniform float a = 3;
uniform int level = 0;

const float pi = 3.1415926535897932384626433832795;
const float pi2 = pi * pi;

float L(float x) {
	if (x == 0) return 1;
	if (x >= a) return 0;

	float t = a * sin(pi * x) * sin (pi * x / a);
	return t / (pi2 * x * x);
}

void main() {
	vec2 coords = gl_FragCoord.xy / outsize * insize;
	float x = dot(filter_dir, coords);

	vec4 c = vec4(0);
	for (int i = floor(x) - a + 1; i <= floor(x) + a; ++i)
		c += L(x - i) * texelFetch(tex, ivec2(coords) + i * filter_dir, level);

	gl_FragColor = c;
}
