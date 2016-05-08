
lll_element lll_encode(uint16_t light_idx, uint16_t ll_idx, float depth_min, float depth_max) {
	lll_element l;
	l.data = vec2(uintBitsToFloat(uint(light_idx) | (uint(ll_idx) << 16)),
				  uintBitsToFloat(packUnorm2x16(vec2(depth_min, depth_max))));
	return l;
}
