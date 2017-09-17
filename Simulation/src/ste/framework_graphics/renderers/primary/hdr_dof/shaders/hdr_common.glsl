
#include <common.glsl>
#include <srgb.glsl>
#include <sfloat_to_uint_order_preserving.glsl>

const int bins = 1024;
const float fbins = float(bins);
const float min_luminance = 1e-9f;


struct hdr_bokeh_parameters {
	uint lum_min;
	uint lum_max;
	float focus;
};

struct bokeh_point_descriptor {
	vec4 pos_size;
	vec4 color;
};


const float scotopic_end = 3e-3f;
const float mesopic_end = 3.f;

const float visual_acuity_coef = .9f;

float scotopic_vision(float lum) {
	return 1.f - clamp((lum - min_luminance) / (scotopic_end - min_luminance), 0.f, 1.f);
}

float mesopic_vision(float lum) {
	return 1.f - clamp((lum - scotopic_end) / (mesopic_end - scotopic_end), 0.f, 1.f);
}

float visual_acuity(float mesopic) {
	const float a = mix(0.f, visual_acuity_coef, mesopic);
	return a*a;
}

float red_response(float scotopic) {
	return mix(1.f, .35f, scotopic);
}

float monochromaticity(float scotopic) {
	return smoothstep(.0f, 1.f, scotopic);
}


float hdr_bin(float max_lum, float min_lum, float l) {
	float range = max_lum - min_lum;
	float bin_size = range / fbins;
	return (l - min_lum) / bin_size;
}

float hdr_lum(float l) {
	return log(l);
}
vec2 hdr_lum(vec2 l) {
	return log(l);
}

float hdr_lum_to_luminance(float l) {
	return exp(l);
}
vec2 hdr_lum_to_luminance(vec2 l) {
	return exp(l);
}

float tonemap(float l) {
	return l;//linear_to_sRGB(l);
}


uint luminance_to_hdr_params_lum(float l) {
	float hdr_lum = hdr_lum(max(min_luminance, l));
	return sfloat_to_uint_order_preserving(hdr_lum);
}

float hdr_lum_from_hdr_params(uint param) {
	return uint_to_sfloat_order_preserving(param);
}
