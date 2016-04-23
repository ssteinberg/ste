
struct g_buffer_element {
	vec4 albedo;
	vec3 P;		uint32_t next_ptr;
	vec3 N;		int32_t material;
	vec3 T;		float specular;
};

layout(std430, binding = 6) buffer gbuffer_data {
	g_buffer_element gbuffer[];
};
layout(binding = 7) uniform atomic_uint gbuffer_ll_counter;
layout(r32ui, binding = 7) uniform uimage2D gbuffer_ll_heads;

void gbuffer_store(vec3 P, vec4 albedo, float specular, vec3 N, vec3 T, int matIdx, ivec2 frag_coords) {
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

bool gbuffer_eof(uint32_t ptr) {
	return ptr == 0xFFFFFFFF;
}

ivec2 gbuffer_size() {
	return imageSize(gbuffer_ll_heads);
}

g_buffer_element gbuffer_load(ivec2 frag_coords) {
	uint32_t idx = imageLoad(gbuffer_ll_heads, frag_coords).x;
	return gbuffer[idx];
}

g_buffer_element gbuffer_load(uint32_t ptr) {
	return gbuffer[ptr];
}

float gbuffer_linear_z(g_buffer_element frag, float far, float near = .0f) {
	return (-frag.P.z - near) / (far - near);
}
