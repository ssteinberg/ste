
#include <common.glsl>

/**
 *	@brief	Converts a color component value from non-linear sRGB colorspace to linear space. 
 *			Uses a standard OETF (Opto-Electrical Transfer Function) with slightly tweaked cutoff value.
 */
float sRGB_to_linear(float srgb) {
	const float sRGB_to_linear_cutoff = 0.0404482362771082;

	if (srgb <= sRGB_to_linear_cutoff)
		return srgb / 12.92f;
	return pow((srgb + 0.055f) / 1.055f, 2.4f);
}

/**
*	@brief	Converts a color component value from linear space to the corresponding electrical signal (sRGB colorspace).
*			Uses a standard OETF (Opto-Electrical Transfer Function) with slightly tweaked cutoff value.
*/
float linear_to_sRGB(float lin) {
	const float linear_to_sRGB_cutoff = 0.00313066844250063;

	if (lin <= linear_to_sRGB_cutoff)
		return lin * 12.92f;
	return 1.055f * pow(lin, 1.f / 2.4f) - 0.055f;
}

/**
 *	@brief	Converts a color component value from non-linear sRGB colorspace to linear space. 
 *			Uses a standard OETF (Opto-Electrical Transfer Function) with slightly tweaked cutoff value.
 */
vec2 sRGB_to_linear(vec2 srgb) {
	const float sRGB_to_linear_cutoff = 0.0404482362771082;

	bvec2 x = greaterThan(srgb, vec2(sRGB_to_linear_cutoff));
	vec2 a = srgb / 12.92f;
	vec2 b = pow((srgb + 0.055f) / 1.055f, vec2(2.4f));
	return mix(a, b, x);
}

/**
*	@brief	Converts a color component value from linear space to the corresponding electrical signal (sRGB colorspace).
*			Uses a standard OETF (Opto-Electrical Transfer Function) with slightly tweaked cutoff value.
*/
vec2 linear_to_sRGB(vec2 lin) {
	const float linear_to_sRGB_cutoff = 0.00313066844250063;
	
	bvec2 x = greaterThan(lin, vec2(linear_to_sRGB_cutoff));
	vec2 a = lin * 12.92f;
	vec2 b = 1.055f * pow(lin, vec2(1.f / 2.4f)) - 0.055f;
	return mix(a, b, x);
}

/**
 *	@brief	Converts a color component value from non-linear sRGB colorspace to linear space. 
 *			Uses a standard OETF (Opto-Electrical Transfer Function) with slightly tweaked cutoff value.
 */
vec3 sRGB_to_linear(vec3 srgb) {
	const float sRGB_to_linear_cutoff = 0.0404482362771082;

	bvec3 x = greaterThan(srgb, vec3(sRGB_to_linear_cutoff));
	vec3 a = srgb / 12.92f;
	vec3 b = pow((srgb + 0.055f) / 1.055f, vec3(2.4f));
	return mix(a, b, x);
}

/**
*	@brief	Converts a color component value from linear space to the corresponding electrical signal (sRGB colorspace).
*			Uses a standard OETF (Opto-Electrical Transfer Function) with slightly tweaked cutoff value.
*/
vec3 linear_to_sRGB(vec3 lin) {
	const float linear_to_sRGB_cutoff = 0.00313066844250063;
	
	bvec3 x = greaterThan(lin, vec3(linear_to_sRGB_cutoff));
	vec3 a = lin * 12.92f;
	vec3 b = 1.055f * pow(lin, vec3(1.f / 2.4f)) - 0.055f;
	return mix(a, b, x);
}

/**
 *	@brief	Converts a color component value from non-linear sRGB colorspace to linear space. 
 *			Uses a standard OETF (Opto-Electrical Transfer Function) with slightly tweaked cutoff value.
 */
vec4 sRGB_to_linear(vec4 srgb) {
	const float sRGB_to_linear_cutoff = 0.0404482362771082;

	bvec4 x = greaterThan(srgb, vec4(sRGB_to_linear_cutoff));
	vec4 a = srgb / 12.92f;
	vec4 b = pow((srgb + 0.055f) / 1.055f, vec4(2.4f));
	return mix(a, b, x);
}

/**
*	@brief	Converts a color component value from linear space to the corresponding electrical signal (sRGB colorspace).
*			Uses a standard OETF (Opto-Electrical Transfer Function) with slightly tweaked cutoff value.
*/
vec4 linear_to_sRGB(vec4 lin) {
	const float linear_to_sRGB_cutoff = 0.00313066844250063;
	
	bvec4 x = greaterThan(lin, vec4(linear_to_sRGB_cutoff));
	vec4 a = lin * 12.92f;
	vec4 b = 1.055f * pow(lin, vec4(1.f / 2.4f)) - 0.055f;
	return mix(a, b, x);
}
