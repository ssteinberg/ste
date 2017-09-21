
/*
 *	@brief	An order-preserving bijection from IEEE-754 32-big signed floating numbers to 32-bit unsigned integers.
 *
 *			For use, e.g., with integer-only atomic operations.
 */
uint sfloat_to_uint_order_preserving(float f) {
	uint ui = floatBitsToUint(f);
	uint mask = uint(-int(ui >> 31) | 0x80000000);
	return ui ^ mask;
}

/*
 *	@brief	The inverse of the order-preserving bijection from IEEE-754 32-big signed floating numbers to 32-bit unsigned integers.
 *			See sfloat_to_uint_order_preserving().
 */
float uint_to_sfloat_order_preserving(uint f) {
	uint mask = ((f >> 31) - 1) | 0x80000000;
	return uintBitsToFloat(f ^ mask);
}
