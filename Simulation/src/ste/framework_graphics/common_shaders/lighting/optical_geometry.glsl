
vec3 snell_refraction(vec3 v,
					  vec3 n,
					  float ior1,
					  float ior2) {
	float r = ior1 / ior2;
	float c = dot(v,n);
	vec3 o = -t*l + (r*c - sqrt(1 - r*r*(1-c*c)))*n;

	return normalize(o);
}
