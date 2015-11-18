
#type frag
#version 450
#extension GL_ARB_bindless_texture : require

#include "hdr_blur.glsl"

out vec4 gl_FragColor;

layout(bindless_sampler) uniform sampler2D hdr;

void main() {
	gl_FragColor = hdr_blur(hdr, ivec2(1,0));
}
