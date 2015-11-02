
#type frag
#version 450

#include "hdr_common.glsl"

layout(location = 0) out vec4 rgbout;
layout(location = 1) out vec4 bloomout;
layout(location = 2) out vec2 coc_out;

layout(binding = 0) uniform sampler2D hdr;
layout(binding = 2) uniform sampler2D z_buffer;
layout(binding = 3) uniform sampler1D hdr_vision_properties_texture;

layout(std430, binding = 0) coherent buffer histogram_sums {
	uint histogram[bins];
};
layout(std430, binding = 2) coherent readonly buffer hdr_bokeh_parameters_buffer {
	hdr_bokeh_parameters params;
};

const float bloom_cutoff = .9f;

uniform float aperature_radius = .025f;
uniform float f1 = .1f;

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
	
	vec4 vision_properties = texture(hdr_vision_properties_texture, clamp((l - min_luminance) / (10.f - min_luminance), 0, 1));
	float scotopic = vision_properties.x;
	float red_coef = vision_properties.y;
	float monochr = vision_properties.z;
	float acuity = vision_properties.w;

	hdr_texel.z = tonemap(toned_l) * mix(1.f, .33f, scotopic);

	vec3 XYZ = xyYtoXYZ(hdr_texel);
	vec3 RGB = XYZtoRGB(XYZ);
	RGB.r *= red_coef;
	RGB = mix(RGB, XYZ.yyy, monochr);
	vec4 RGBL = clamp(vec4(RGB, XYZ.y), vec4(0), vec4(1));

	rgbout = RGBL;
	
	if (XYZ.y > bloom_cutoff) {
		float x = pow((XYZ.y - bloom_cutoff) / (1 - bloom_cutoff), 8);
		bloomout = vec4(RGBL.rgb, x);
	}
	else
		bloomout = vec4(0);

	float focal = params.focus;
	float s = texelFetch(z_buffer, ivec2(gl_FragCoord.xy), 0).x;
	float C = aperature_radius * abs(focal - s) / s;
	float c = C * f1 / focal;
	float coc = clamp(smoothstep(0, 1, c), 0, 1);
	coc += acuity;
	
	coc_out = vec2(s, clamp(coc, 0, 1));
}
