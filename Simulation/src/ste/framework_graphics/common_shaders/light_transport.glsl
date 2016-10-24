
#include "common.glsl"

float henyey_greenstein_phase_function(vec3 l_dir, vec3 v_dir, float g) {
	float cosine = dot(l_dir, v_dir);
	float g2 = g*g;

	float denom = four_pi * pow(1.f + g2 - 2.f * g * cosine, 3.f / 2.f);

	return (1.f - g2) / denom;
}

vec3 beer_lambert(vec3 att, float path_length) {
	// Limit thickness to flt_min to avoid NaN when attenuation coefficient is infinite
	return exp(-max(flt_min, path_length) * att);
}
