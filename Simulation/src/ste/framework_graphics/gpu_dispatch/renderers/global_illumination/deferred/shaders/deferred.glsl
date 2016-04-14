
layout(location = 0) out vec3 o_frag_position;
layout(location = 1) out vec4 o_frag_color;
layout(location = 2) out vec3 o_frag_normal;
layout(location = 3) out float o_frag_z;
layout(location = 4) out vec3 o_frag_tangent;
layout(location = 5) out int o_frag_mat_idx;
layout(location = 6) out vec3 o_frag_wposition;

void deferred_output(vec3 P, vec3 w_pos, vec3 diffuse, float specular, vec3 N, vec3 T, float Z, int matIdx) {
	o_frag_tangent = T;
	o_frag_normal = N;
	o_frag_color = vec4(diffuse, specular);

	o_frag_position = P;
	o_frag_wposition = w_pos;
	o_frag_z = Z;
	o_frag_mat_idx = matIdx;
}
