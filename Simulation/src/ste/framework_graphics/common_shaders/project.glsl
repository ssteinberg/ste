
float unproject_depth(float d, float proj23) {
	return -proj23 / d;
}

float project_depth(float z, float proj23) {
	return -proj23 / z;
}

vec3 unproject_screen_position(float depth, vec2 norm_frag_coords, float proj23, float proj00, float proj11) {
	norm_frag_coords *= 2.f;
	norm_frag_coords -= vec2(1);
	vec3 p = vec3(norm_frag_coords, depth);

	float z = proj23 / p.z;
	vec2 xy = (p.xy * z) / vec2(proj00, proj11);

	return vec3(xy, -z);
}
