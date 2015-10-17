
#type frag
#version 440

in vec3 l_pos;

out vec4 gl_FragColor;

layout(binding = 0) uniform sampler2D normal_tex;
layout(binding = 1) uniform sampler2D position_tex;
layout(binding = 3) uniform sampler2D color_tex;

const float l_intensity = 1000;
const float Ka = .2;
const float Kd = .8;

float l_calc(vec3 n, vec3 p) {
	vec3 s = normalize(vec3(l_pos) - p);

	return l_intensity * (Ka +
						  Kd * max(dot(s, n), .0));
}

void main() {
	vec3 normal = texelFetch(normal_tex, ivec2(gl_FragCoord.xy), 0).xyz;
	vec3 position = texelFetch(position_tex, ivec2(gl_FragCoord.xy), 0).xyz;
	vec3 color = texelFetch(color_tex, ivec2(gl_FragCoord.xy), 0).rgb;

	color.z *= l_calc(normal, position);

	gl_FragColor = vec4(color, 1);
}
