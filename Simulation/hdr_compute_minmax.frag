
#type frag
#version 450

out float gl_FragColor;

#include "hdr_common.glsl"

layout(binding = 0) uniform sampler2D hdr;
layout(binding = 0, r32i) uniform iimage2D out_histogram_minmax;

in vec2 tex_coords;

void main() {
	float texel = textureLod(hdr, tex_coords, 0).z;

	float l = hdr_lum(texel);
		
	int int_l = floatBitsToInt(l);
	imageAtomicMin(out_histogram_minmax, ivec2(0, 0), int_l);
	imageAtomicMax(out_histogram_minmax, ivec2(1, 0), int_l);

	gl_FragColor = l;
}
