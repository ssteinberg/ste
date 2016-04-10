
float shadow_obscurance(samplerCube shadow_depth_map, vec3 w_pos, vec3 l_pos, float dist) {
	vec3 shadow_v = (w_pos - l_pos);
	float l_shadow_v = dist / 3000.f;
	float shadow_d = texture(shadow_depth_map, shadow_v).x;
	if (shadow_d < l_shadow_v - .00005f)
		return .1f;

	return 1.f;
}