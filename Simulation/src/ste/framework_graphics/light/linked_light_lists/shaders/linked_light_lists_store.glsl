
#include "linked_light_lists.glsl"

lll_element lll_encode(uint16_t light_idx, uint16_t ll_idx, vec2 depth_range) {
	lll_element l;

	uint idx = uint(light_idx) | (uint(ll_idx) << 16);
	uint dr = packUnorm2x16(depth_range);

	l.data = uvec2(idx, dr);

	return l;
}
