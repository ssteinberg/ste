
#include <common.glsl>

/*
 *	Does an intersection test of an open sphere with a frustum.
 *
 *	Frustum is defined by 6 planes whose equation is given, i.e. p.xyz is the plane normal and p.w the distance 
 *	from origin. Given point x and plane p: dot(p.xyz, x) == -p.w iff x lies on the plane.
 *
 *	The supplied planes must have the normal pointing into the frustum.
 *
 *	@param c		Sphere center
 *	@param r		Sphere radius
 *	@param np		Near plane equation
 *	@param fp		Far plane equation
 *	@param rp		Right plane equation
 *	@param lp		Left plane equation
 *	@param tp		Top plane equation
 *	@param bp		Bottom plane equation
 */
bool collision_sphere_frustum(vec3 c, 
							  float r,
							  vec4 np,
							  vec4 fp,
							  vec4 rp,
							  vec4 lp,
							  vec4 tp,
							  vec4 bp) {
	return  dot(np.xyz, c) + np.w > -r &&
			dot(rp.xyz, c) + rp.w > -r &&
			dot(lp.xyz, c) + lp.w > -r &&
			dot(tp.xyz, c) + tp.w > -r &&
			dot(bp.xyz, c) + bp.w > -r &&
			dot(fp.xyz, c) + fp.w > -r;
}

/*
 *	Does an intersection test of an open sphere with an infinite frustum.
 *
 *	See collision_sphere_frustum
 *
 *	@param c		Sphere center
 *	@param r		Sphere radius
 *	@param np		Near plane equation
 *	@param rp		Right plane equation
 *	@param lp		Left plane equation
 *	@param tp		Top plane equation
 *	@param bp		Bottom plane equation
 */
bool collision_sphere_infinite_frustum(vec3 c, 
									   float r,
									   vec4 np,
									   vec4 rp,
									   vec4 lp,
									   vec4 tp,
									   vec4 bp) {
	return  dot(np.xyz, c) + np.w > -r &&
			dot(rp.xyz, c) + rp.w > -r &&
			dot(lp.xyz, c) + lp.w > -r &&
			dot(tp.xyz, c) + tp.w > -r &&
			dot(bp.xyz, c) + bp.w > -r;
}

/*
 *	Does an intersection test of an open sphere with another open sphere.
 *
 *	@param c1		Sphere 1 center
 *	@param r1		Sphere 1 radius
 *	@param c2		Sphere 2 center
 *	@param r2		Sphere 2 radius
 */
bool collision_sphere_sphere(vec3 c1, float r1,
							 vec3 c2, float r2) {
	vec3 v = c1 - c2;
	float r = r1 + r2;

	return dot(v,v) < r*r;
}

/*
 *	Calculates intersection distance between a ray and a sphere. 
 *	Returns +inf if does not intersect.
 *
 *	@param c		Sphere center
 *	@param r		Sphere radius
 *	@param o		Ray origin
 *	@param l		Ray direction
 */
float intersection_ray_sphere(vec3 c, float r,
							  vec3 o, vec3 l) {
	vec3 t = o - c;
	float lent = length(t);
	float x = dot(l, t);
	float d = x*x - lent*lent + r*r;
	if (d < 0)
		return +inf;

	vec2 dist = -x.xx + vec2(1,-1) * sqrt(d);
	bvec2 dist_less_then_zero = lessThan(dist, vec2(0));
	if (all(dist_less_then_zero))
		return +inf;
	if (any(dist_less_then_zero))
		return max_element(dist);
	return min_element(dist);
}
