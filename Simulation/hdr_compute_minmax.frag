
#type frag
#version 450

out float gl_FragColor;

#include "hdr_common.glsl"

layout(binding = 0) uniform sampler2D hdr;

layout(std430, binding = 2) coherent buffer hdr_bokeh_parameters_buffer {
	hdr_bokeh_parameters params;
};

in vec2 tex_coords;

void main() {
	float texel = textureLod(hdr, tex_coords, 0).z;

	float l = hdr_lum(texel);
		
	int int_l = floatBitsToInt(l);

	atomicMin(params.lum_min, int_l);
	atomicMax(params.lum_max, int_l);

	gl_FragColor = l;
}
