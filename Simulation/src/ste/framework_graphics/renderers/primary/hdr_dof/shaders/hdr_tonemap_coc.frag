
#type frag
#version 450

#include <chromaticity.glsl>
#include <hdr_common.glsl>

layout(binding = 0) uniform sampler2D hdr;
layout(binding = 1) uniform sampler2D hdr_vision_properties_texture;

layout(std430, binding = 2) restrict readonly buffer histogram_sums {
	uint histogram[bins];
};
layout(std430, binding = 3) restrict readonly buffer hdr_bokeh_parameters_buffer {
	hdr_bokeh_parameters params;
};

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 rgbout;
layout(location = 1) out vec4 bloomout;

const float bloom_cutoff = .666f;
const float vision_properties_max_lum = 10.f;

vec4 hdr_bloom(vec3 rgb, float luminance, float mesopic) {
	if (luminance > bloom_cutoff) {
		float x = pow((luminance - bloom_cutoff) / (1.f - bloom_cutoff), 8) * (1.f - mesopic);
		return vec4(rgb, x);
	}
	else
		return vec4(0);
}

float hdr_tonemap(float luminance) {
	const float T = float(histogram[bins - 1]);

	float l = hdr_lum(luminance);
	
	// Read bins-range (logarithmic) luminance and linear luminance
	float min_lum = hdr_lum_from_hdr_params(params.lum_min);
	float max_lum = hdr_lum_from_hdr_params(params.lum_max);

	// Compute bin for incoming luminance
	float fbin = max(hdr_bin(max_lum, min_lum, l), .0f);
	int bin = int(fbin);
	if (bin >= bins)
		return tonemap(1.f);

	// Calculate linear luminance on bin's low and high-end
	uint toned_bin_start = bin > 0 ? histogram[bin - 1] : 0;
	uint toned_bin_end = histogram[bin];
	vec2 bin_range = vec2(toned_bin_start, toned_bin_end) / T;

	vec2 bin_hdr_lum_range = mix(min_lum.xx, max_lum.xx, vec2(bin, bin+1) / fbins);
	vec2 bin_luminance_range = hdr_lum_to_luminance(bin_hdr_lum_range);

	// Calculate relative position inside bin
	float frac = (luminance - bin_luminance_range.x) / (bin_luminance_range.y - bin_luminance_range.x);

	// Tonemap
	float toned_l = mix(bin_range.x, bin_range.y, frac);
	return tonemap(toned_l);
}

void main() {
	vec3 xyY = texelFetch(hdr, ivec2(gl_FragCoord.xy), 0).rgb;
	float x = xyY.z;

	// Read precomputed vision properties
	float vision_properties_coord = (x - min_luminance) / (vision_properties_max_lum - min_luminance);
	vec2 vision_properties = texture(hdr_vision_properties_texture, vec2(vision_properties_coord, .5f)).xy;
	float scotopic = vision_properties.x;
	float mesopic = vision_properties.y;
	float red_coef = red_response(scotopic);
	float monochrm = monochromaticity(scotopic);

	// Red response
	vec3 rgb = XYZtoRGB(xyYtoXYZ(xyY));
	rgb.r *= red_coef;
	xyY = XYZtoxyY(RGBtoXYZ(rgb));

	// Tonemap
	float tonemapped_luminance = hdr_tonemap(x);// * mix(1.f, .666f, mesopic);

	xyY.z = tonemapped_luminance;
	rgb = XYZtoRGB(xyYtoXYZ(xyY));

	// Monochromaticity
	rgb = mix(rgb, tonemapped_luminance.xxx, monochrm);
	vec4 rgb_lum = clamp(vec4(rgb, tonemapped_luminance), 0.f, 1.f);

	// Bloom
	vec4 bloom = hdr_bloom(rgb_lum.rgb, tonemapped_luminance, .0f);

	// Write output
	rgbout = rgb_lum;
	bloomout = bloom;
}
