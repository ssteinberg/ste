
vec4 project(vec4 proj_xywz, vec4 v) {
	return v.xywz * proj_xywz;
}

float project_depth(float z, float n) {
	return -n / z;
}

float unproject_depth(float d, float n) {
	return -n / d;
}

float project_depth(float z, float n, float f) {
	float t = 1.f/(f-n);
	return -n*t * (f/z + 1.f);
}

float unproject_depth(float d, float n, float f) {
	float t = f-n;
	return -n*f / (t*d + n);
}

vec3 unproject_screen_position(float depth, vec2 norm_frag_coords, vec4 proj_xywz) {
	norm_frag_coords *= 2.f;
	norm_frag_coords -= vec2(1);

	float z = unproject_depth(depth, proj_xywz.z);
	vec2 xy = (norm_frag_coords * z) / vec2(proj_xywz.xy);

	return vec3(-xy, z);
}

vec3 unproject_screen_position_with_z(float z, vec2 norm_frag_coords, vec4 proj_xywz) {
	norm_frag_coords *= 2.f;
	norm_frag_coords -= vec2(1);

	vec2 xy = (norm_frag_coords * z) / vec2(proj_xywz.xy);

	return vec3(-xy, z);
}
