
#include "common.glsl"

bool snell_refraction(inout vec3 v,
					  vec3 n,
					  float ior1,
					  float ior2) {
	vec3 t = cross(n, -v);
	float ior = ior1 / ior2;

	float cosine2 = 1.f - ior * ior * dot(t, t);
	if (cosine2 < .0f)
		return false;

	vec3 x = n * sqrt(cosine2);
	v = x - ior * cross(n, -t);

	return true;
}

float henyey_greenstein_phase_function(vec3 l_dir, vec3 v_dir, float g) {
	float cosine = dot(l_dir, v_dir);
	float g2 = g*g;

	float denom = 4.f * pi * pow(1.f + g2 - 2.f * g * cosine, 3.f / 2.f);

	return (1.f - g2) / denom;
}
