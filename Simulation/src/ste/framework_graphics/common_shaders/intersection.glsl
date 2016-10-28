
/*
 *	Does an intersection test of a sphere with a frustum.
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
 *	Does an intersection test of a sphere with an infinite frustum.
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
