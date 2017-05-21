
#include <linked_light_lists.glsl>

ivec2 lll_get_size(restrict readonly uimage2D lll_heads) {
	return imageSize(lll_heads);
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
