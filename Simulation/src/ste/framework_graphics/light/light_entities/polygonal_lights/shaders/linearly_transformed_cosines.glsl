
#include "common.glsl"

vec2 ltc_lut_coords(sampler2D ltc_lut, float cos_theta, float roughness) {
	float theta = acos(cos_theta);
	vec2 coords = vec2(roughness, theta / half_pi);

	const vec2 size = vec2(256.f);// textureSize(ltc_lut, 0);
	// scale and bias coordinates, for correct filtered lookup
	coords = coords*(size - vec2(1.f))/size + .5f/size;

	return coords;
}

mat3 ltc_inv_matrix(sampler2D ltc_lut, vec2 coord) {
	vec4 t = texture(ltc_lut, coord);
	return mat3(
		vec3(1.f, .0f, t.y),
		vec3(.0f, t.z, .0f),
		vec3(t.w, .0f, t.x)
	);
}

float ltc_integrate_edge(vec3 v1, vec3 v2) {
	float cos_theta = dot(v1, v2);
	cos_theta = clamp(cos_theta, -0.9999f, 0.9999f);

	float theta = acos(cos_theta);    
	float res = cross(v1, v2).z * theta / sin(theta);

	return res;
}

// Clips a quad to the horizon, returning 4 or 5 vertices.
void ltc_clip_quad(inout vec3 L[5], out int n) {
	int config = 0;
	if (L[0].z > 0.0) config += 1;
	if (L[1].z > 0.0) config += 2;
	if (L[2].z > 0.0) config += 4;
	if (L[3].z > 0.0) config += 8;

	n = 0;
	if (config == 1) { // V1 clip V2 V3 V4
		n = 3;
		L[1] = -L[1].z * L[0] + L[0].z * L[1];
		L[2] = -L[3].z * L[0] + L[0].z * L[3];
	}
	else if (config == 2) { // V2 clip V1 V3 V4
		n = 3;
		L[0] = -L[0].z * L[1] + L[1].z * L[0];
		L[2] = -L[2].z * L[1] + L[1].z * L[2];
	}
	else if (config == 3) { // V1 V2 clip V3 V4
		n = 4;
		L[2] = -L[2].z * L[1] + L[1].z * L[2];
		L[3] = -L[3].z * L[0] + L[0].z * L[3];
	}
	else if (config == 4) { // V3 clip V1 V2 V4
		n = 3;
		L[0] = -L[3].z * L[2] + L[2].z * L[3];
		L[1] = -L[1].z * L[2] + L[2].z * L[1];
	}
	else if (config == 6) { // V2 V3 clip V1 V4
		n = 4;
		L[0] = -L[0].z * L[1] + L[1].z * L[0];
		L[3] = -L[3].z * L[2] + L[2].z * L[3];
	}
	else if (config == 7) { // V1 V2 V3 clip V4
		n = 5;
		L[4] = -L[3].z * L[0] + L[0].z * L[3];
		L[3] = -L[3].z * L[2] + L[2].z * L[3];
	}
	else if (config == 8) { // V4 clip V1 V2 V3
		n = 3;
		L[0] = -L[0].z * L[3] + L[3].z * L[0];
		L[1] = -L[2].z * L[3] + L[3].z * L[2];
		L[2] =  L[3];
	}
	else if (config == 9) { // V1 V4 clip V2 V3
		n = 4;
		L[1] = -L[1].z * L[0] + L[0].z * L[1];
		L[2] = -L[2].z * L[3] + L[3].z * L[2];
	}
	else if (config == 11) { // V1 V2 V4 clip V3
		n = 5;
		L[4] = L[3];
		L[3] = -L[2].z * L[3] + L[3].z * L[2];
		L[2] = -L[2].z * L[1] + L[1].z * L[2];
	}
	else if (config == 12) { // V3 V4 clip V1 V2
		n = 4;
		L[1] = -L[1].z * L[2] + L[2].z * L[1];
		L[0] = -L[0].z * L[3] + L[3].z * L[0];
	}
	else if (config == 13) { // V1 V3 V4 clip V2
		n = 5;
		L[4] = L[3];
		L[3] = L[2];
		L[2] = -L[1].z * L[2] + L[2].z * L[1];
		L[1] = -L[1].z * L[0] + L[0].z * L[1];
	}
	else /*if (config == 14)*/ { // V2 V3 V4 clip V1
		n = 5;
		L[4] = -L[0].z * L[3] + L[3].z * L[0];
		L[0] = -L[0].z * L[1] + L[1].z * L[0];
	}
	
	if (n == 3)
		L[3] = L[0];
	if (n == 4)
		L[4] = L[0];
}

// Clips a triangle to the horizon, returning 3 or 4 vertices.
void ltc_clip_triangle(inout vec3 L[4], out int n) {
	int config = 0;
	if (L[0].z > 0.0) config += 1;
	if (L[1].z > 0.0) config += 2;
	if (L[2].z > 0.0) config += 4;

	n = 0;
	if (config == 1) { // V1 clip V2 V3
		n = 3;
		L[1] = -L[1].z * L[0] + L[0].z * L[1];
		L[2] = -L[2].z * L[0] + L[0].z * L[2];
	}
	else if (config == 2) { // V2 clip V1 V3
		n = 3;
		L[0] = -L[0].z * L[1] + L[1].z * L[0];
		L[2] = -L[2].z * L[1] + L[1].z * L[2];
	}
	else if (config == 3) { // V1 V2 clip V3
		n = 4;
		L[3] = -L[2].z * L[0] + L[0].z * L[2];
		L[2] = -L[2].z * L[1] + L[1].z * L[2];
	}
	else if (config == 4) { // V3 clip V1 V2
		n = 3;
		L[0] = -L[0].z * L[2] + L[2].z * L[0];
		L[1] = -L[1].z * L[2] + L[2].z * L[1];
	}
	else if (config == 5) { // V1 V3 clip V2
		n = 4;
		L[3] = L[2];
		L[2] = -L[2].z * L[1] + L[1].z * L[2];
		L[1] = -L[1].z * L[0] + L[0].z * L[1];
	}
	else /*if (config == 6)*/ { // V2 V3 clip V1
		n = 4;
		L[3] = -L[0].z * L[2] + L[2].z * L[0];
		L[0] = -L[0].z * L[1] + L[1].z * L[0];
	}
	
	if (n == 3)
		L[3] = L[0];
}

/*
 *	Integrates irradiance of a quad projected onto a sphere with clamped cosine distribution.
 *	
 *	The input is a quad, which is then clipped to the upper hemisphere of point P with normal N.
 *	Input is read from storage buffer ltc_points.
 *
 *	@param N			Normal, world coordinates.
 *	@param V			Position to eye vector, normalized.
 *	@param P			Position, world coordinates.
 *	@param Minv			Inverse of M, linear transformation matrix from clamped cosine distribution.
 *	@param light_pos	Light position, world coordinates.
 *	@param offset		Offset into point buffer (ltc_points).
 *	@param two_sided	Single or two-sided shape. Must be false for polyhedrons.
 */
vec3 ltc_evaluate_quad(vec3 N, 
					   vec3 V, 
					   vec3 P, 
					   mat3 Minv, 
					   vec3 light_pos, 
					   uint offset, 
					   bool two_sided) {//, sampler2D texFilteredMap) 
	// construct orthonormal basis around N
	vec3 T1, T2;
	T1 = normalize(V - N*dot(V, N));
	T2 = cross(N, T1);

	// rotate area light in (T1, T2, R) basis
	Minv = Minv * transpose(mat3(T1, T2, N));

	// polygon (allocate 5 vertices for clipping)
	vec3 L[5];
	L[0] = Minv * (light_pos + ltc_points[offset + 0].xyz - P);
	L[1] = Minv * (light_pos + ltc_points[offset + 1].xyz - P);
	L[2] = Minv * (light_pos + ltc_points[offset + 2].xyz - P);
	L[3] = Minv * (light_pos + ltc_points[offset + 3].xyz - P);
	L[4] = L[0];

	vec3 textureLight = vec3(1, 1, 1);
	//textureLight = FetchDiffuseFilteredTexture(texFilteredMap, L[0], L[1], L[2], L[3]);

	vec4 zs = vec4(L[0].z, L[1].z, L[2].z, L[3].z);
	bool all_below_horizon = all(lessThanEqual(zs, vec4(0)));
	bool all_above_horizon = all(greaterThanEqual(zs, vec4(0)));

	int n;
	if (all_below_horizon)
		return vec3(.0f);
	else if (all_above_horizon)
		n = 4;
	else
		ltc_clip_quad(L, n);

	// project onto sphere
	L[0] = normalize(L[0]);
	L[1] = normalize(L[1]);
	L[2] = normalize(L[2]);
	L[3] = normalize(L[3]);
	L[4] = normalize(L[4]);

	// integrate
	float sum = 0.0;
	sum += ltc_integrate_edge(L[0], L[1]);
	sum += ltc_integrate_edge(L[1], L[2]);
	sum += ltc_integrate_edge(L[2], L[3]);
	if (n >= 4)
		sum += ltc_integrate_edge(L[3], L[4]);
	if (n == 5)
		sum += ltc_integrate_edge(L[4], L[0]);

	sum = two_sided ? 
			abs(sum) : 
			max(.0f, -sum);

	vec3 Lo_i = vec3(sum);

	// scale by filtered light color
	Lo_i *= textureLight;

	return Lo_i / two_pi;
}

/*
 *	Integrates polygonal irradiance projected onto a sphere with clamped cosine distribution.
 *	
 *	The input is an arbitary polygon or a convex polyhedron, constructed from triangles. In the latter 
 *	case the light can not be two-sided. The input is clipped to the upper hemisphere of point P with normal N.
 *	Input is read from storage buffer ltc_points.
 *
 *	@param N			Normal, world coordinates.
 *	@param V			Position to eye vector, normalized.
 *	@param P			Position, world coordinates.
 *	@param Minv			Inverse of M, linear transformation matrix from clamped cosine distribution.
 *	@param light_pos	Light position, world coordinates.
 *	@param primitives	Primitives (triangles) count.
 *	@param offset		Offset into point buffer (ltc_points).
 *	@param two_sided	Single or two-sided shape. Must be false for polyhedrons.
 */
vec3 ltc_evaluate_polygon(vec3 N, 
						  vec3 V, 
						  vec3 P, 
						  mat3 Minv, 
						  vec3 light_pos, 
						  uint primitives, 
						  uint offset, 
						  bool two_sided) {//, sampler2D texFilteredMap)
	// construct orthonormal basis around N
	vec3 T1, T2;
	T1 = normalize(V - N*dot(V, N));
	T2 = cross(N, T1);

	// rotate area light in (T1, T2, R) basis
	Minv = Minv * transpose(mat3(T1, T2, N));

	vec3 accum = vec3(.0f);
	for (int t=0; t<primitives; ++t) {
		// polygon (allocate 4 vertices for clipping)
		vec3 L[4];
		L[0] = Minv * (light_pos + ltc_points[offset + 3*t + 0].xyz - P);
		L[1] = Minv * (light_pos + ltc_points[offset + 3*t + 1].xyz - P);
		L[2] = Minv * (light_pos + ltc_points[offset + 3*t + 2].xyz - P);

		vec3 textureLight = vec3(1, 1, 1);
		//textureLight = FetchDiffuseFilteredTexture(texFilteredMap, L[0], L[1], L[2], L[3]);
	
		vec3 zs = vec3(L[0].z, L[1].z, L[2].z);
		bool all_below_horizon = all(lessThanEqual(zs, vec3(.0f)));
		bool all_above_horizon = all(greaterThanEqual(zs, vec3(.0f)));

		int n;
		if (all_below_horizon)
			continue;
		else if (all_above_horizon)
			n = 3;
		else
			ltc_clip_triangle(L, n);

		// project onto sphere
		L[0] = normalize(L[0]);
		L[1] = normalize(L[1]);
		L[2] = normalize(L[2]);
		L[3] = normalize(L[3]);

		// integrate
		float sum = 0.0;

		sum += ltc_integrate_edge(L[0], L[1]);
		sum += ltc_integrate_edge(L[1], L[2]);
		sum += ltc_integrate_edge(L[2], L[3]);
		if (n == 4)
			sum += ltc_integrate_edge(L[3], L[0]);

		// note: negated due to winding order
		sum = two_sided ? 
				abs(sum) : 
				max(.0f, -sum);

		// scale by filtered light color
		accum += sum * textureLight;
	}

	vec3 Lo_i = vec3(accum);

	return Lo_i / two_pi;
}

/*
 *	Integrates irradiance of a sphere projected onto a sphere with clamped cosine distribution.
 *	
 *	The input is a sphere, which is then clipped to the upper hemisphere of point P with normal N.
 *
 *	@param N			Normal, world coordinates.
 *	@param V			Position to eye vector, normalized.
 *	@param P			Position, world coordinates.
 *	@param Minv			Inverse of M, linear transformation matrix from clamped cosine distribution.
 *	@param light_pos	Light position, world coordinates.
 *	@param r			Sphere radius.
 */
vec3 ltc_evaluate_sphere(vec3 N, 
						 vec3 V, 
						 vec3 P, 
						 mat3 Minv, 
						 vec3 light_pos, 
						 float r) {
	const float points_per_solid_angle = 45.f;

	// construct orthonormal basis around N
	vec3 T1, T2;
	T1 = normalize(V - N*dot(V, N));
	T2 = cross(N, T1);

	// rotate area light in (T1, T2, R) basis
	Minv = Minv * transpose(mat3(T1, T2, N));

	float sum = .0f;
	vec3 l = light_pos - P;
	
	// Compute solid angle, and use it to select amount of circle integration points
	float solid_angle = clamp(r / length(l), epsilon, 2);
	float n = 2.f + solid_angle * points_per_solid_angle;
	float two_pi_recp_n = two_pi / n;

	// As the polygon inscribed in the circle has lower area then the circle, compensate with larger radius
	// Area of a regular n-vertex polygon inscribed in a circle is: .5*r^2*n*sin(2pi/n)
	float d = max(1.f, sqrt(half_pi / (n * sin(two_pi_recp_n))));
	r *= d;

	vec3 t = cross(vec3(1,0,0), l);
	if (dot(t,t) < 1e-5f)
		t = cross(vec3(0,0,1), l);
	t = normalize(t);
	vec3 b = normalize(cross(l, t));

	t *= r;
	b *= r;
	
	// Integrate
	vec3 start_point = Minv * (l + t);
	start_point.z = max(.0f, start_point.z);
	start_point = normalize(start_point);

	vec3 p0 = start_point;
	for (float i=1; i<n; ++i) {
		float x = i * two_pi_recp_n;

		vec3 p1 = l + cos(x) * t + sin(x) * b;
		p1 = Minv * p1;
		p1.z = max(.0f, p1.z);
		p1 = normalize(p1);
		
		sum += ltc_integrate_edge(p0, p1);

		p0 = p1;
	}
	
	sum += ltc_integrate_edge(p0, start_point);

	return abs(sum).xxx / two_pi;
}
