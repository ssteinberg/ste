
#include <common.glsl>

const int bins = 128;
const float fbins = float(bins);
const float min_luminance = .000001f;

struct hdr_bokeh_parameters {
	int lum_min, lum_max;
	float focus;
};

struct bokeh_point_descriptor {
	vec4 pos_size;
	vec4 color;
};

float hdr_scotopic(float lum) {
	return 1.f - clamp((lum - min_luminance) / (.1f - min_luminance), .0f, 1.f);
}

float hdr_mesopic(float lum) {
	return 1.f - clamp((lum - .01f) / (10.f - .01f), .0f, 1.f);
}

float hdr_acuity(float lum) {
	float a = mix(0.f, .7f, hdr_scotopic(lum));
	float a2 = a * a;
	return a2 * a2;
}

float red_response(float mesopic) {
	return mix(1.f, .35f, mesopic);
}

float monochromaticity(float lum) {
	return smoothstep(1.f, .0f, clamp((lum - min_luminance) / (.025f - min_luminance), .0f, 1.f));
}

float hdr_bin(float max_lum, float min_lum, float l) {
	float range = max_lum - min_lum;
	float bin_size = range / fbins;
	float r = (l - min_lum) / bin_size;
	return clamp(r, 0.f, fbins - .0000001f);
}

float hdr_lum(float l) {
	return log(l + 1.f);// / log(10.f);
}

float hdr_lum_to_luminance(float l) {
	return exp(l) - 1.f;
}

float tonemap(float l) {
	return l;
}
