
#include "linked_light_lists.glsl"

bool lll_eof(lll_element l) {
	return l.light_idx == uint16_t(0xFFFF);
}

ivec2 lll_size(layout(r32ui) restrict readonly uimage2D lll_heads) {
	return imageSize(lll_heads);
}

bool lll_load(uint32_t ptr, out lll_element l) {
	l = lll_buffer[ptr];
	if (lll_eof(l))
		return false;
	return true;
}
