
#include "gbuffer.glsl"
#include "pack.glsl"

void gbuffer_store(layout(r32ui) restrict uimage2D gbuffer_ll_heads,
				   atomic_uint gbuffer_ll_counter,
				   float depth,
				   float alpha,
				   vec2 UV,
				   vec2 dUVdx,
				   vec2 dUVdy,
				   vec3 N,
				   vec3 T,
				   int matIdx,
				   ivec2 frag_coords) {
	uint32_t next_idx = atomicCounterIncrement(gbuffer_ll_counter);
	uint32_t next_ptr = imageAtomicExchange(gbuffer_ll_heads, frag_coords, next_idx);

	float dUVdx16 = uintBitsToFloat(packFloat2x16(f16vec2(dUVdx)));
	float dUVdy16 = uintBitsToFloat(packFloat2x16(f16vec2(dUVdy)));

	uint Npack = packSnorm2x16(normal3x32_to_snorm2x32(N));
	uint Tpack = packSnorm2x16(normal3x32_to_snorm2x32(T));

	mat3x4 data;
	data[0] = vec4(depth, uintBitsToFloat(next_ptr), uintBitsToFloat(matIdx), alpha);
	data[1] = vec4(UV, uintBitsToFloat(Npack), uintBitsToFloat(Tpack));
	data[2].xy = vec2(dUVdx16, dUVdy16);

	gbuffer[next_idx].data = data;
}

void gbuffer_write_nextptr(inout g_buffer_element frag, uint32_t next_ptr) {
	frag.data[0].y = uintBitsToFloat(next_ptr);
}
