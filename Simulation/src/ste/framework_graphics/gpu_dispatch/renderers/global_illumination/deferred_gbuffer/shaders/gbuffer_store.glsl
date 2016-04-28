
#include "gbuffer.glsl"

void gbuffer_store(layout(r32ui) restrict uimage2D gbuffer_ll_heads,
				   atomic_uint gbuffer_ll_counter,
				   vec3 P,
				   vec4 albedo,
				   float specular,
				   vec3 N,
				   vec3 T,
				   uint16_t matIdx,
				   ivec2 frag_coords) {
	g_buffer_element frag;
	frag.albedo = albedo;
	frag.P = P;
	frag.N = N;
	frag.T = T;
	frag.specular = specular;
	frag.material = matIdx;

	uint32_t next_idx = atomicCounterIncrement(gbuffer_ll_counter);
	uint32_t prev_head = imageAtomicExchange(gbuffer_ll_heads, frag_coords, next_idx);

	frag.next_ptr = prev_head;

	gbuffer[next_idx] = frag;
}
