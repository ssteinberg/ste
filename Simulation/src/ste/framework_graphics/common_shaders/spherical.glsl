

vec3 vec_from_spherical(float theta, float phi, float r = 1.f) {
	float sine_theta = sin(theta);
	return vec3(r * sine_theta * cos(phi), r * sine_theta * sin(phi), r * cos(theta));
}

void spherical_from_vec(vec3 v, out float theta, out float phi, out float r) {
	float x2y2 = v.x*v.x + v.y*v.y;

	r = sqrt(x2y2 + v.z*v.z);
	theta = atan(sqrt(x2y2) / v.z);
	phi = atan(v.y, v.x);
}

void spherical_from_norm_vec(vec3 v, out float theta, out float phi) {
	float x2y2 = 1.f - v.z*v.z;

	theta = atan(sqrt(x2y2) / v.z);
	phi = atan(v.y, v.x);
}
