
#type frag
#version 450

const int blur_samples_count = 8;

#include <hdr_blur.glsl>

layout(binding = 0) uniform sampler2D hdr;

layout(location = 0) out vec4 frag_color;

void main() {
	frag_color = hdr_blur(hdr, vec2(1.f, .0f));
}
