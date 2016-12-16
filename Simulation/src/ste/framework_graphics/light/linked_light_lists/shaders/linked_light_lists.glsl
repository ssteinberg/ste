
struct lll_element {
	uvec2 data;
};

const uint lll_eof_marker = 0xFFFFFFFF;

/*
 *	Size in pixels of a single per-pixel linked-light-list. 
 */
const int lll_image_res_multiplier = 8;

/*
 *	Minimal luminance of low detail light. Multiplier is used to adjust the default minimal luminance of a light to compute its new effective range.
 */
const float lll_low_detail_min_luminance_multiplier = 5.f;

/*
 *	Maximum amount of low detail lights in a linked-list, per pixel. 
 */
const int max_active_low_detail_lights_per_frame = max_active_lights_per_frame >> 2;
