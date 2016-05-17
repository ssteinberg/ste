
vec4 project(vec4 proj_xywz, vec4 v) {
	return v.xywz * proj_xywz;
}

float project_depth(float z, float n) {
	return -n / z;
}

float unproject_depth(float d, float n) {
	return -n / d;
}

vec3 unproject_screen_position(float depth, vec2 norm_frag_coords, vec4 proj_xywz) {
	norm_frag_coords *= 2.f;
	norm_frag_coords -= vec2(1);
	vec3 p = vec3(norm_frag_coords, depth);

	float z = unproject_depth(p.z, proj_xywz.z);
	vec2 xy = (p.xy * z) / vec2(proj_xywz.xy);

	return vec3(-xy, z);
}
