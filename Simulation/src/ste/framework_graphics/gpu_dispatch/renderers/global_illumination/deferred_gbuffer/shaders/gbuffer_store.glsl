
#include "gbuffer.glsl"
#include "pack.glsl"

void gbuffer_store(layout(r32ui) restrict uimage2D gbuffer_ll_heads,
				   atomic_uint gbuffer_ll_counter,
				   float depth,
				   vec2 UV,
				   f16vec2 dUVdx,
				   f16vec2 dUVdy,
				   vec3 N,
				   vec3 T,
				   int matIdx,
				   ivec2 frag_coords) {
	uint32_t next_idx = atomicCounterIncrement(gbuffer_ll_counter);
	uint32_t next_ptr = imageAtomicExchange(gbuffer_ll_heads, frag_coords, next_idx);

	float dUVdx16 = uintBitsToFloat(packFloat2x16(dUVdx));
	float dUVdy16 = uintBitsToFloat(packFloat2x16(dUVdy));

	uvec3 Npack = snorm12x2_to_unorm8x3(float32x3_to_oct(N));
	uvec3 Tpack = snorm12x2_to_unorm8x3(float32x3_to_oct(T));

	uint32_t NTpack0 = Npack.x + (Npack.y << 8) + (Npack.z << 16) + (Tpack.z << 24);
	uint16_t NTpack1 = uint16_t(Tpack.x + (Tpack.y << 8));
	uint16_t mat16 = uint16_t(matIdx);

	float enc0 = uintBitsToFloat(NTpack0);
	float enc1 = uintBitsToFloat(NTpack1 + (mat16 << 16));

	mat2x4 data;
	data[0] = vec4(depth, UV, uintBitsToFloat(next_ptr));
	data[1] = vec4(enc0, enc1, dUVdx16, dUVdy16);

	gbuffer[next_idx].data = data;
}

void gbuffer_write_nextptr(inout g_buffer_element frag, uint32_t next_ptr) {
	frag.data[0].w = uintBitsToFloat(next_ptr);
}
