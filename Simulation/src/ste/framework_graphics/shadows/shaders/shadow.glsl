
float shadow_obscurance(samplerCubeArray shadow_depth_maps, int light, vec3 w_pos, vec3 l_pos, float dist, float far) {
	vec3 shadow_v = (w_pos - l_pos);
	float l_shadow_v = dist / far;
	float shadow_d = texture(shadow_depth_maps, vec4(shadow_v, light)).x;
	if (shadow_d < l_shadow_v - .00005f)
		return .0f;

	return 1.f;
}
