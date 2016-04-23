
#type frag
#version 450
#extension GL_NV_gpu_shader5 : require

#include "gbuffer.glsl"

in vec2 frag_texcoords;
in vec3 frag_position;
in float frag_depth;

layout(binding = 0) uniform sampler2D sky_tex;

out vec4 gl_FragColor;

uniform float sky_luminance;

void main() {
	vec2 uv = frag_texcoords;
	vec3 P = frag_position;

	vec4 diffuse = vec4(texture(sky_tex, uv).rgb * sky_luminance, 1);
	int matIdx = -1;

	gbuffer_store(P, diffuse, .0f, vec3(0,0,0), vec3(0,0,0), matIdx, ivec2(gl_FragCoord.xy));
}
