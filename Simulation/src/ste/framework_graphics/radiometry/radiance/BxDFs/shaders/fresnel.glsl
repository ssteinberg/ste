
float fresnel_F0(float sin_critical) {
	float t = (1.f - sin_critical) / (1.f + sin_critical);
	return t * t;
}

float fresnel_schlick_ratio(float cos_theta_incident) {
	float p = 1.f - cos_theta_incident;
	float p2 = p*p;
	return p2 * p2 * p;
}

float fresnel_schlick_ratio(float cos_theta_incident, float power) {
	float p = 1.f - cos_theta_incident;
	return pow(p, power);
}

float fresnel_schlick(float F0, float cos_theta_incident) {
	return mix(F0, 1.f, fresnel_schlick_ratio(cos_theta_incident));
}

float fresnel_schlick_tir(float F0, float cos_theta_incident, float cos_critical) {
	if (cos_theta_incident <= cos_critical)
		return 1.f;

	float p = 1.f - (cos_theta_incident - cos_critical) / (1 - cos_critical);
	float p2 = p*p;
	float a = p2 * p2;

	return mix(F0, 1.f, a);
}

float fresnel_steinberg(float F0, float cos_theta_incident, float cos_critical, float sin_critical) {
	if (cos_theta_incident <= cos_critical)
		return 1.f;

	float p = 1.f - (cos_theta_incident - cos_critical) / (1 - cos_critical);
	float a = pow(p, 6.f + 18.f*exp(-13.f*max(.0f, sin_critical - 1)));
	return mix(F0, 1.f, a);
}

float fresnel(float cos_theta_incident, float cos_critical, float sin_critical) {
	if (cos_theta_incident <= cos_critical)
		return 1.f;

	float sin_theta_incident2 = 1.f - cos_theta_incident*cos_theta_incident;
	float t = sqrt(1.f - sin_theta_incident2 / (sin_critical * sin_critical));
	float sqrtRs = (cos_theta_incident - sin_critical * t) / (cos_theta_incident + sin_critical * t);
	float sqrtRp = (t - sin_critical * cos_theta_incident) / (t + sin_critical * cos_theta_incident);

	return mix(sqrtRs * sqrtRs, sqrtRp * sqrtRp, .5f);
}
