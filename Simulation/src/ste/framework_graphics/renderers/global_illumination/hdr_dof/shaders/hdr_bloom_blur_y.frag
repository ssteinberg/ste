
#type frag
#version 450
#extension GL_ARB_bindless_texture : require

const int blur_samples_count = 8;

#include <hdr_blur.glsl>

out vec3 gl_FragColor;

layout(bindless_sampler) uniform sampler2D unblured_hdr;
layout(bindless_sampler) uniform sampler2D hdr;

uniform vec2 size;
uniform vec2 dir;

void main() {
	vec4 blur = hdr_blur(hdr, size, dir);

	vec4 hdr_texel = texelFetch(unblured_hdr, ivec2(gl_FragCoord.xy), 0);
	vec3 blend = blur.rgb * blur.a + hdr_texel.rgb;
	gl_FragColor = blend;
}
