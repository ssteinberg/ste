
/*
 *	@brief	Stevens's power law is a proposed relationship between the magnitude of a physical stimulus and its perceived intensity.
 *			Returns k*I^a
 */
float stevens_power_law(float I, float k, float a) {
	return k * pow(I, a);
}

/*
 *	@brief	Stevens's power law is a proposed relationship between the magnitude of a physical stimulus and its perceived intensity.
 *			Returns k*I^a
 */
vec2 stevens_power_law(vec2 I, float k, float a) {
	return k * pow(I, a.xx);
}

/*
 *	@brief	Stevens's power law is a proposed relationship between the magnitude of a physical stimulus and its perceived intensity.
 *			Returns k*I^a
 */
vec3 stevens_power_law(vec3 I, float k, float a) {
	return k * pow(I, a.xxx);
}

/*
 *	@brief	Stevens's power law is a proposed relationship between the magnitude of a physical stimulus and its perceived intensity.
 *			Returns k*I^a
 */
vec4 stevens_power_law(vec4 I, float k, float a) {
	return k * pow(I, a.xxxx);
}

/*
 *	@brief	Stevens's power law is a proposed relationship between the magnitude of a physical stimulus and its perceived intensity.
 *			Returns the inverse of k*I^a
 */
float inverse_stevens_power_law(float p, float k, float a) {
	return pow(p / k, 1.f / a);
}

/*
 *	@brief	Stevens's power law is a proposed relationship between the magnitude of a physical stimulus and its perceived intensity.
 *			Returns the inverse of k*I^a
 */
vec2 inverse_stevens_power_law(vec2 p, float k, float a) {
	return pow(p / k, 1.f / a.xx);
}

/*
 *	@brief	Stevens's power law is a proposed relationship between the magnitude of a physical stimulus and its perceived intensity.
 *			Returns the inverse of k*I^a
 */
vec3 inverse_stevens_power_law(vec3 p, float k, float a) {
	return pow(p / k, 1.f / a.xxx);
}

/*
 *	@brief	Stevens's power law is a proposed relationship between the magnitude of a physical stimulus and its perceived intensity.
 *			Returns the inverse of k*I^a
 */
vec4 inverse_stevens_power_law(vec4 p, float k, float a) {
	return pow(p / k, 1.f / a.xxxx);
}
