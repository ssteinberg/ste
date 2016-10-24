
/*
 *	Computes approximates the error function with 2.5*10^(-5) error bound.
 *	From C. Hastings Jr., Approximations for digital computers. Princeton Univ. Press, Princeton, N.J., 1955
 *	http://people.math.sfu.ca/~cbm/aands/page_299.htm
 */
float erf(float x) {
	const float p = .47047f;
	const float a1 = .3480242f;
	const float a2 = -.0958798f;
	const float a3 = .7478556f;

	float t = 1.f / (1.f + p*abs(x));
	float t2 = t*t;
	float t3 = t2*t;

	return sign(x) * (1.f - (a1*t + a2*t2 + a3*t3)*exp(-x*x));
}
