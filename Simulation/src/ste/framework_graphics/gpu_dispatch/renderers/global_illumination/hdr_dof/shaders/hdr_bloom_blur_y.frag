
#type frag
#version 450
#extension GL_ARB_bindless_texture : require

#include "hdr_blur.glsl"

out mediump vec4 gl_FragColor;

layout(bindless_sampler) uniform sampler2D unblured_hdr;
layout(bindless_sampler) uniform sampler2D hdr;

void main() {
	mediump vec4 blur = hdr_blur(hdr, ivec2(0,1));

	mediump vec4 hdr_texel = texelFetch(unblured_hdr, ivec2(gl_FragCoord.xy), 0);
	mediump vec3 blend = blur.rgb * blur.a + hdr_texel.rgb;
	gl_FragColor = vec4(blend, hdr_texel.w + .1f);
}
