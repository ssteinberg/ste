
#type frag
#version 450

const int blur_samples_count = 8;

#include <hdr_blur.glsl>

in vs_out {
	vec2 uv;
	vec2 blur_uvs[blur_samples_count];
} vin;

layout(location = 0) uniform sampler2D hdr;

uniform size_t {
	vec2 size;
};
uniform dir_t {
	vec2 dir;
};

out vec4 frag_color;

void main() {
	frag_color = hdr_blur(hdr, size, dir);
}
