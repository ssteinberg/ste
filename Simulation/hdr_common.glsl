
#include "common.glsl"

const int bins = 128;
const float fbins = float(bins);
const float min_luminance = .0001f;

struct hdr_bokeh_parameters {
	int lum_min, lum_max;
	float focus;
};

struct bokeh_point_descriptor {
	vec4 pos_size;
	vec4 color;
};

float hdr_bin(float max_lum, float min_lum, float l) {
	float range = max_lum - min_lum;
	float bin_size = range / fbins;
	float r = (l - min_lum) / bin_size;
	return clamp(r, 0.f, fbins - .000001f);
}

float hdr_lum(float l) {
	return max(l, min_luminance);
}

float tonemap(float l) {
	return l;
}
