
float shadow_penumbra_width(samplerCubeArray shadow_depth_maps, int light, vec3 shadow_v, float l_radius, float dist, float far, out bool shadowed) {
	float l_shadow_v = dist / far; // length(shadow_v) / far;

	float shadow_d = texture(shadow_depth_maps, vec4(shadow_v, light)).x;

	float d = l_shadow_v - shadow_d;
	shadowed = d > 1.f / 1000.f;

	return (d / shadow_d) * l_radius;
}
