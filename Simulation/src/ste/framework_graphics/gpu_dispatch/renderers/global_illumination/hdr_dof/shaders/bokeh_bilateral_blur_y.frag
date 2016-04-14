
#type frag
#version 450
#extension GL_ARB_bindless_texture : require

#include "hdr_blur.glsl"

out mediump vec4 gl_FragColor;

layout(bindless_sampler) uniform sampler2D hdr;
layout(bindless_sampler) uniform sampler2D zcoc_buffer;

void main() {
	gl_FragColor = bokeh_blur(hdr, zcoc_buffer, ivec2(0,1));
}
