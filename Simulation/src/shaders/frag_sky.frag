
#type frag
#version 450
#extension GL_NV_gpu_shader5 : require

layout(early_fragment_tests) in;

#include "gbuffer.glsl"

in vec2 frag_texcoords;
in vec3 frag_position;
in vec3 frag_wposition;

layout(binding = 0) uniform sampler2D sky_tex;

uniform float sky_luminance;

void main() {
	vec2 uv = frag_texcoords;
	vec3 P = frag_position;

	vec4 diffuse = vec4(texture(sky_tex, uv).rgb * sky_luminance, 1);
	uint16_t matIdx = uint16_t(0xFFFF);

	gbuffer_store(P, diffuse, .0f, vec3(0,0,0), vec3(0,0,0), matIdx, ivec2(gl_FragCoord.xy));
}
