
#include "linked_light_lists.glsl"

ivec2 lll_size(layout(r32ui) restrict readonly uimage2D lll_heads) {
	return imageSize(lll_heads);
}

float lll_parse_zmin(lll_element l) {
	return l.data.z;
}

float lll_parse_zmax(lll_element l) {
	return l.data.w;
}

uint lll_parse_light_idx(lll_element l) {
	return floatBitsToUint(l.data.x);
}

uint lll_parse_ll_idx(lll_element l) {
	return floatBitsToUint(l.data.y);
}

bool lll_eof(lll_element l) {
	return lll_parse_light_idx(l) == 0xFFFFFFFF;
}
