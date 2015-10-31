
#type frag
#version 440

#include "hdr_common.glsl"

layout(location = 0) out vec4 rgbout;
layout(location = 1) out vec4 bloomout;

layout(binding = 0) uniform sampler2D hdr;

layout(std430, binding = 0) coherent buffer histogram_sums {
	uint histogram[bins];
};
layout(std430, binding = 2) coherent readonly buffer hdr_bokeh_parameters_buffer {
	hdr_bokeh_parameters params;
};

const float bloom_cutoff = .9f;

void main() {
	vec3 hdr_texel = texelFetch(hdr, ivec2(gl_FragCoord.xy), 0).rgb;

	float min_lum = intBitsToFloat(params.lum_min);
	float max_lum = intBitsToFloat(params.lum_max);

	float l = hdr_lum(hdr_texel.z);
	float fbin = hdr_bin(max_lum, min_lum, l);
	int bin = int(fbin);
	float frac = fract(fbin);
	
	float T = float(histogram[bins - 1]);
	float toned_bin_start = bin > 0 ? float(histogram[bin - 1]) / T : .0f;
	float toned_bin_end = float(histogram[bin]) / T;
	float toned_bin_size = toned_bin_end - toned_bin_start;

	float toned_l = toned_bin_start + frac * toned_bin_size;

	hdr_texel.z = tonemap(toned_l);

	vec3 XYZ = xyYtoXYZ(hdr_texel);
	vec4 RGBL = clamp(vec4(XYZtoRGB(XYZ), XYZ.y), vec4(0), vec4(1));

	rgbout = RGBL;
	
	if (XYZ.y > bloom_cutoff) {
		float x = pow((XYZ.y - bloom_cutoff) / (1 - bloom_cutoff), 8);
		bloomout = vec4(RGBL.rgb, x);
	}
	else
		bloomout = vec4(0);
}
