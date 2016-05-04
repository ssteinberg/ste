
#include "gbuffer.glsl"

void gbuffer_store(layout(r32ui) restrict uimage2D gbuffer_ll_heads,
				   atomic_uint gbuffer_ll_counter,
				   vec3 P,
				   float alpha,
				   float specular,
				   vec2 UV,
				   f16vec3 N,
				   f16vec3 T,
				   int matIdx,
				   ivec2 frag_coords) {
	uint32_t next_idx = atomicCounterIncrement(gbuffer_ll_counter);
	uint32_t next_ptr = imageAtomicExchange(gbuffer_ll_heads, frag_coords, next_idx);

	float Nxy = uintBitsToFloat(packFloat2x16(N.xy));
	float NzTx = uintBitsToFloat(packFloat2x16(f16vec2(N.z, T.x)));
	float Tyz = uintBitsToFloat(packFloat2x16(T.yz));

	mat3x4 data;
	data[0] = vec4(P, uintBitsToFloat(next_ptr));
	data[1] = vec4(Nxy, NzTx, Tyz, intBitsToFloat(matIdx));
	data[2] = vec4(UV, alpha, specular);

	gbuffer[next_idx].data = data;
}

void gbuffer_write_nextptr(inout g_buffer_element frag, uint32_t next_ptr) {
	frag.data[0].w = uintBitsToFloat(next_ptr);
}
