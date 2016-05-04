
#type frag
#version 450
#extension GL_ARB_bindless_texture : require

const int blur_samples_count = 8;

#include "hdr_blur.glsl"

out vec4 gl_FragColor;

in vs_out {
	vec2 uv;
	vec2 blur_uvs[blur_samples_count];
} vin;

layout(bindless_sampler) uniform sampler2D unblured_hdr;
layout(bindless_sampler) uniform sampler2D hdr;

void main() {
	vec4 blur = hdr_blur(hdr, vin.uv, vin.blur_uvs);

	vec4 hdr_texel = texelFetch(unblured_hdr, ivec2(gl_FragCoord.xy), 0);
	vec3 blend = blur.rgb * blur.a + hdr_texel.rgb;
	gl_FragColor = vec4(blend, hdr_texel.w + .1f);
}
