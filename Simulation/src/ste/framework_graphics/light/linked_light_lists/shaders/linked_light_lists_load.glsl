
#include "linked_light_lists.glsl"

bool lll_eof(uint32_t ptr) {
	return ptr == 0xFFFFFFFF;
}

ivec2 lll_size(layout(r32ui) restrict readonly uimage2D lll_heads) {
	return imageSize(lll_heads);
}

bool lll_load(uint32_t ptr, out lll_element l) {
	if (lll_eof(ptr))
		return false;
	l = lll_buffer[ptr];
	return true;
}

bool lll_load(layout(r32ui) restrict readonly uimage2D lll_heads, ivec2 frag_coords, out lll_element l) {
	return lll_load(imageLoad(lll_heads, frag_coords).x, l);
}
