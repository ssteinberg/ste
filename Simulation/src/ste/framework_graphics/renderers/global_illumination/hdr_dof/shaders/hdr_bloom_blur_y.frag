
#type frag
#version 450

const int blur_samples_count = 8;

#include <hdr_blur.glsl>

layout(binding = 0) uniform sampler2D unblured_hdr;
layout(binding = 1) uniform sampler2D hdr;

layout(push_constant) uniform config_t {
	vec2 dir;
};

layout(location = 0) out vec3 frag_color;

void main() {
	vec4 blur = hdr_blur(hdr, dir);

	vec4 hdr_texel = texelFetch(unblured_hdr, ivec2(gl_FragCoord.xy), 0);
	vec3 blend = blur.rgb * blur.a + hdr_texel.rgb;
	frag_color = blend;
}
