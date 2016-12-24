
#include "light.glsl"

struct lll_element {
	uvec2 data;
};

/*
 *	Size in pixels of a single per-pixel linked-light-list. 
 */
const int lll_image_res_multiplier = 8;
const int lll_depth_lod = 2;	// = log2(lll_image_res_multiplier)-1  (Down-sampled depth starts from LOD 1)

/*
 *	Minimal luminance of low detail light. Multiplier is used to adjust the default minimal luminance of a light to compute its new effective range.
 */
const float lll_low_detail_min_luminance_multiplier = 6.f;

/*
 *	Maximum amount of low detail lights in a linked-list, per pixel. 
 */
const int max_active_low_detail_lights_per_frame = total_max_active_lights_per_frame >> 1;

float lll_low_detail_light_minimal_luminance(light_descriptor ld) {
	return light_calculate_minimal_luminance(ld) * lll_low_detail_min_luminance_multiplier;
}

float lll_low_detail_light_effective_range(light_descriptor ld) {
	float min_lum = lll_low_detail_light_minimal_luminance(ld);
	return light_calculate_effective_range(ld, min_lum);
}
