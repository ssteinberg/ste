
/*
 *	Projects a eye space position to homogeneous clip coordinates. Uses infinite depth projection.
 */
vec4 project(vec4 proj_xywz, vec4 v) {
	return v.xywz * proj_xywz;
}

/*
 *	Projects a z value to a depth value. Uses infinite depth projection.
 */
float project_depth(float z, float n) {
	return -n / z;
}

/*
 *	Unprojects a depth value to the z value. Uses infinite depth projection.
 */
float unproject_depth(float d, float n) {
	return -n / d;
}

/*
 *	Projects a z value to a depth value
 */
float project_depth(float z, float n, float f) {
	float t = 1.f/(f-n);
	return -n*t * (f/z + 1.f);
}

/*
 *	Unprojects a depth value to the z value
 */
float unproject_depth(float d, float n, float f) {
	float t = f-n;
	return -n*f / (t*d + n);
}

/*
 *	Linearly projects a z value to a depth value
 */
float project_depth_linear(float z, float n, float f) {
	return (f + z) / (f - n);
}

/*
 *	Unprojects a linear depth value to the z value
 */
float unproject_depth_linear(float d, float n, float f) {
	return d * (f - n) - f;
}

/*
*	Unprojects a screen position, given with depth value and normalized screen coordinates, into eye space
*/
vec3 unproject_screen_position(float depth, vec2 norm_frag_coords, vec4 proj_xywz) {
	norm_frag_coords *= 2.f;
	norm_frag_coords -= vec2(1);

	float z = unproject_depth(depth, proj_xywz.z);
	vec2 xy = (norm_frag_coords * z) / vec2(proj_xywz.xy);

	return vec3(-xy, z);
}

/*
*	Unprojects a screen position, given with a eye space z value and normalized screen coordinates, into eye space
*/
vec3 unproject_screen_position_with_z(float z, vec2 norm_frag_coords, vec4 proj_xywz) {
	norm_frag_coords *= 2.f;
	norm_frag_coords -= vec2(1);

	vec2 xy = (norm_frag_coords * z) / vec2(proj_xywz.xy);

	return vec3(-xy, z);
}
