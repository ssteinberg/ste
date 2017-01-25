
/*
 *	Fresnel reflectance at 0 angle incident
 */
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

/*
 *	Fresnel reflection Schlick's approximation
 *
 *	@param F0					Fresnel reflectance at 0 angle incident
 *	@param cos_theta_incident	Dot product of incident vector and normal
 */
float fresnel_schlick(float F0, float cos_theta_incident) {
	return mix(F0, 1.f, fresnel_schlick_ratio(cos_theta_incident));
}

/*
 *	Fresnel reflection Schlick's approximation respecting total-internal-reflection
 *
 *	@param F0					Fresnel reflectance at 0 angle incident
 *	@param cos_theta_incident	Dot product of incident vector and normal
 *	@param cos_critical			Cosine of critical angle
 */
float fresnel_schlick_tir(float F0, float cos_theta_incident, float cos_critical) {
	if (cos_theta_incident <= cos_critical)
		return 1.f;

	float p = 1.f - (cos_theta_incident - cos_critical) / (1 - cos_critical);
	float p2 = p*p;
	float a = p2 * p2;

	return mix(F0, 1.f, a);
}

/*
 *	Fresnel reflection approximation respecting total-internal-reflection and providing slightly better
 *	fitting to Fresnel equations than Schlick's approximation
 *
 *	@param F0					Fresnel reflectance at 0 angle incident
 *	@param cos_theta_incident	Dot product of incident vector and normal
 *	@param cos_critical			Cosine of critical angle, [0, 1]
 *	@param refractive_ratio		Ratio of refractive-indices, ior2/ior1
 */
float fresnel_steinberg(float F0, float cos_theta_incident, float cos_critical, float refractive_ratio) {
	if (cos_theta_incident <= cos_critical)
		return 1.f;

	float p = 1.f - (cos_theta_incident - cos_critical) / (1 - cos_critical);
	float a = pow(p, 6.f + 18.f*exp(-13.f*max(.0f, refractive_ratio - 1)));
	return mix(F0, 1.f, a);
}

/*
 *	Fresnel reflection using Fresnel equations
 *
 *	@param cos_theta_incident	Dot product of incident vector and normal
 *	@param cos_critical			Cosine of critical angle, [0, 1]
 *	@param refractive_ratio		Ratio of refractive-indices, ior2/ior1
 */
float fresnel(float cos_theta_incident, float cos_critical, float refractive_ratio) {
	if (cos_theta_incident <= cos_critical)
		return 1.f;

	float sin_theta_incident2 = 1.f - cos_theta_incident*cos_theta_incident;
	float t = sqrt(1.f - sin_theta_incident2 / (refractive_ratio * refractive_ratio));
	float sqrtRs = (cos_theta_incident - refractive_ratio * t) / (cos_theta_incident + refractive_ratio * t);
	float sqrtRp = (t - refractive_ratio * cos_theta_incident) / (t + refractive_ratio * cos_theta_incident);

	return mix(sqrtRs * sqrtRs, sqrtRp * sqrtRp, .5f);
}
