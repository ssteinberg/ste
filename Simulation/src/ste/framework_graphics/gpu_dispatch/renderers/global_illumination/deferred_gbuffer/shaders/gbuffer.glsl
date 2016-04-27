
struct g_buffer_element {
	vec3 P;				uint32_t next_ptr;
	mediump vec3 N, T;	mediump float specular; uint16_t material;
	mediump vec4 albedo;
};
