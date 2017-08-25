
#include <light.glsl>

struct lll_element {
	uvec2 data;
};

layout(r8ui,   set=2, binding=2) restrict uniform uimage2D linked_light_list_size;
layout(r8ui,   set=2, binding=3) restrict uniform uimage2D linked_light_list_low_detail_size;
layout(r32ui,  set=2, binding=4) restrict uniform uimage2D linked_light_list_heads;
layout(r32ui,  set=2, binding=5) restrict uniform uimage2D linked_light_list_low_detail_heads;

layout(std430, set=2, binding=9) restrict buffer linked_light_list_counter_binding {
	uint lll_counter;
};
layout(std430, set=2, binding=10) restrict buffer linked_light_list_binding {
	lll_element lll_buffer[];
};

/*
 *	Size in pixels of a single per-pixel linked-light-list. 
 */
const int lll_image_res_multiplier = 8;
const int lll_depth_lod = 2;	// = log2(lll_image_res_multiplier)-1  (Down-sampled depth starts from LOD 1)

/*
 *	Minimal luminance of low detail light. Multiplier is used to adjust the default minimal luminance of a light to compute its new effective range.
 */
const float lll_low_detail_min_luminance_multiplier = 55.f;

/*
 *	Maximum amount of low detail lights in a linked-list, per pixel. 
 */
const int max_active_low_detail_lights_per_frame = total_max_active_lights_per_frame >> 1;


float lll_low_detail_light_effective_range(light_descriptor ld) {
	return light_effective_range(ld) / sqrt(lll_low_detail_min_luminance_multiplier);
}

ivec2 lll_get_size() {
	return imageSize(linked_light_list_heads);
}

vec2 lll_parse_depth_range(lll_element l) {
	uint zpack = l.data.y;
	return unpackUnorm2x16(zpack);
}

uint lll_parse_light_idx(lll_element l) {
	uint pack = l.data.x;
	return pack & 0xFFFF;
}

uint lll_parse_ll_idx(lll_element l) {
	uint pack = l.data.x;
	return pack >> 16;
}
