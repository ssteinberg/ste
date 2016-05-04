
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

layout(bindless_sampler) uniform sampler2D hdr;
layout(bindless_sampler) uniform sampler2D zcoc_buffer;

void main() {
	gl_FragColor = bokeh_blur(hdr, zcoc_buffer, vin.uv, vin.blur_uvs);
}
