
float interleaved_gradient_noise(vec2 seed) {
	vec3 magic = vec3(0.06711056f, 0.00583715f, 52.9829189f);
	return fract(magic.z * fract(dot(seed, magic.xy)));
}
