
lll_element lll_encode(uint light_idx, uint ll_idx, float z_min, float z_max) {
	lll_element l;
	l.data = vec4(uintBitsToFloat(light_idx),
				  uintBitsToFloat(ll_idx),
				  z_min,
				  z_max);
	return l;
}
