
#type frag
#version 440

#include "hdr_common.glsl"

layout(location = 0) out vec4 rgbout;
layout(location = 1) out vec4 bloomout;

layout(binding = 0) uniform sampler2D hdr;
layout(binding = 0, r32i) uniform readonly iimage2D histogram_minmax;

layout(std430, binding = 0) coherent buffer histogram_sums {
	uint histogram[bins];
};

const float bloom_cutoff = .85f;

void main() {
	vec3 hdr_texel = texelFetch(hdr, ivec2(gl_FragCoord.xy), 0).rgb;
	float min_lum = intBitsToFloat(imageLoad(histogram_minmax, ivec2(0, 0)).x);
	float max_lum = intBitsToFloat(imageLoad(histogram_minmax, ivec2(1, 0)).x);

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

	vec3 XYZ;
	float Y_y = hdr_texel.z / hdr_texel.y;
	XYZ.x = Y_y * hdr_texel.x;
	XYZ.z = Y_y * (1 - hdr_texel.x - hdr_texel.y);
	XYZ.y = hdr_texel.z;

	vec3 RGB = clamp(XYZtoRGB * XYZ, vec3(0,0,0), vec3(1,1,1));

	rgbout = vec4(RGB, XYZ.y);
	
	if (XYZ.y > bloom_cutoff) {
		float x = pow((XYZ.y - bloom_cutoff) / (1 - bloom_cutoff), 6);
		bloomout = vec4(RGB, x);
	}
	else
		bloomout = vec4(0);
}
