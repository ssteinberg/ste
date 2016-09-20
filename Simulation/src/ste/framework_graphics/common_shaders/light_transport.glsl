
#include "common.glsl"

float henyey_greenstein_phase_function(vec3 l_dir, vec3 v_dir, float g) {
	float cosine = dot(l_dir, v_dir);
	float g2 = g*g;

	float denom = 4.f * pi * pow(1.f + g2 - 2.f * g * cosine, 3.f / 2.f);

	return (1.f - g2) / denom;
}
