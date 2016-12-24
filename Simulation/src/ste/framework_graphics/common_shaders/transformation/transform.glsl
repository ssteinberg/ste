
/*
 *	Compute a lookat transformation matrix.
 *
 *	{up, direction, tangent} must be an orthonormal basis.
 *	If unavailable use helper method look_at(eye, center, up).
 *
 *	@param eye		Eye position
 *	@param center	Point where the camera is looking at
 *	@param up		Normalized up vector, controls camera orientation.
 *	@param direction	Normalized direction of view, must be normalize(eye - center).
 *	@param tangent	Normalized tangent.
 */
mat4 look_at(vec3 eye, vec3 center, vec3 up,
			 vec3 direction, vec3 tangent) {
	vec3 f = -direction;
	vec3 s = tangent;
	vec3 u = up;

	mat4 m = mat4(1.f);
	m[0][0] = s.x;
	m[1][0] = s.y;
	m[2][0] = s.z;
	m[0][1] = u.x;
	m[1][1] = u.y;
	m[2][1] = u.z;
	m[0][2] = f.x;
	m[1][2] = f.y;
	m[2][2] = f.z;
	m[3][0] = -dot(s, eye);
	m[3][1] = -dot(u, eye);
	m[3][2] = -dot(f, eye);

	return m;
}

/*
 *	Compute a lookat transformation matrix.
 *
 *	@param eye		Eye position
 *	@param center	Point where the camera is looking at
 *	@param up		Normalized up vector, controls camera orientation.
 */
mat4 look_at(vec3 eye, vec3 center, vec3 up) {
	vec3 f = normalize(center - eye);
	vec3 s = normalize(cross(up, f));
	vec3 u = cross(f, s);

	return look_at(eye, center, u, f, s);
}

/*
 *	Compute a reversed projection matrix.
 *
 *	@param fovy		Full field of view
 *	@param aspect	Width to height aspect ratio
 *	@param n		Near clipping plane
 *	@param f		Far clipping plane
 */
mat4 perspective_reversed(float fovy, float aspect, float n, float f) {
	float tanHalfFovy = tan(fovy * .5f);

	mat4 m = mat4(.0f);

	m[0][0] = 1.f / (aspect * tanHalfFovy);
	m[1][1] = 1.f / tanHalfFovy;
	m[2][2] = n / (f - n);
	m[2][3] = -1.f;
	m[3][2] = n * f / (f - n);

	return m;
}

/*
 *	Compute an infinite reversed projection matrix.
 *
 *	@param fovy		Full field of view
 *	@param aspect	Width to height aspect ratio
 *	@param n		Near clipping plane
 */
mat4 perspective_reversed_infinite(float fovy, float aspect, float n) {
	float tanHalfFovy = tan(fovy * .5f);

	mat4 m = mat4(.0f);

	m[0][0] = 1.f / (aspect * tanHalfFovy);
	m[1][1] = 1.f / tanHalfFovy;
	m[2][3] = -1.f;
	m[3][2] = n;

	return m;
}

/*
 *	Compute an orthographic projection matrix.
 *
 *	@param top		Upper limit on Y axis
 *	@param bottom	Lower limit on Y axis
 *	@param right	Upper limit on X axis
 *	@param left		Lower limit on X axis
 *	@param near		Near clip plane
 *	@param far		Far clip plane
 */
mat4 ortho(float top, float bottom, float right, float left, float near, float far) {
	mat4 m = mat4(1.f);

	m[0][0] =   2.f / (right - left);
	m[1][1] =   2.f / (top - bottom);
	m[3][0] = - (right + left) / (right - left);
	m[3][1] = - (top + bottom) / (top - bottom);
	
	m[2][2] = - 2.f / (far - near);
	m[3][2] = - (far + near) / (far - near);

	return m;
}

/*
 *	Compute a symmetric orthographic projection matrix.
 *	i.e. l = -r, b = -t.
 *
 *	@param top		Absolute limit on Y axis
 *	@param right	Absolute limit on X axis
 *	@param near		Near clip plane
 *	@param far		Far clip plane
 */
mat4 ortho(float top, float right, float near, float far) {
	mat4 m = mat4(1.f);

	m[0][0] =   1.f / right;
	m[1][1] =   1.f / top;
	m[2][2] = - 2.f / (far - near);
	m[3][2] = - (far + near) / (far - near);

	return m;
}
