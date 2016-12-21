
#include "linked_light_lists.glsl"

ivec2 lll_get_size(layout(r32ui) restrict readonly uimage2D lll_heads) {
	return imageSize(lll_heads);
}

vec2 lll_parse_depth_range(lll_element l) {
	uint zpack = l.data.y;
	return unpackUnorm2x16(zpack);
}

uint16_t lll_parse_light_idx(lll_element l) {
	uint pack = l.data.x;
	return uint16_t(pack & 0xFFFF);
}

uint16_t lll_parse_ll_idx(lll_element l) {
	uint pack = l.data.x;
	return uint16_t(pack >> 16);
}
