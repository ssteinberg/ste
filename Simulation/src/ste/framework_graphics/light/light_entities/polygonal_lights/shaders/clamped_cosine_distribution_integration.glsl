
#include "common.glsl"
#include "linearly_transformed_cosines.glsl"

/*
 *	Integrates spherical clamped cosine distribution of a projected quad.
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
	vec3 v[5];
	v[0] = Minv * (light_pos + ltc_point(ltc_points[offset + 0]) - P);
	v[1] = Minv * (light_pos + ltc_point(ltc_points[offset + 1]) - P);
	v[2] = Minv * (light_pos + ltc_point(ltc_points[offset + 2]) - P);
	v[3] = Minv * (light_pos + ltc_point(ltc_points[offset + 3]) - P);
	v[4] = v[0];

	//vec3 textureLight = FetchDiffuseFilteredTexture(texFilteredMap, v[0], v[1], v[2], v[3]);

	vec4 zs = vec4(v[0].z, v[1].z, v[2].z, v[3].z);
	bool all_below_horizon = all(lessThanEqual(zs, vec4(0)));
	bool all_above_horizon = all(greaterThan(zs, vec4(0)));

	int n;
	if (all_below_horizon)
		return vec3(.0f);
	else if (all_above_horizon)
		n = 4;
	else {
		ltc_clip_quad(v, n);
		if (n==0) return vec3(0);
	}

	// project onto sphere
	v[0] = normalize(v[0]);
	v[1] = normalize(v[1]);
	v[2] = normalize(v[2]);
	v[3] = normalize(v[3]);
	v[4] = normalize(v[4]);

	// integrate
	float sum = 0.0;
	sum += ltc_integrate_edge(v[0], v[1]);
	sum += ltc_integrate_edge(v[1], v[2]);
	sum += ltc_integrate_edge(v[2], v[3]);
	if (n >= 4)
		sum += ltc_integrate_edge(v[3], v[4]);
	if (n == 5)
		sum += ltc_integrate_edge(v[4], v[0]);

	sum = two_sided ? 
			abs(sum) : 
			max(.0f, -sum);

	return vec3(sum) / two_pi;
}

/*
 *	Integrates spherical clamped cosine distribution of a projected polygon cross section.
 *	
 *	The input is an arbitary (planar) polygon. The input is clipped to the upper hemisphere of point P with normal N.
 *	Input is read from storage buffer ltc_points.
 *
 *	@param N			Normal, world coordinates.
 *	@param V			Position to eye vector, normalized.
 *	@param P			Position, world coordinates.
 *	@param Minv			Inverse of M, linear transformation matrix from clamped cosine distribution.
 *	@param light_pos	Light position, world coordinates.
 *	@param points		Point count. Must be >2.
 *	@param offset		Offset into point buffer (ltc_points).
 *	@param two_sided	Single or two-sided shape. Must be false for polyhedrons.
 */
vec3 ltc_evaluate_polygon(vec3 N, 
						  vec3 V, 
						  vec3 P, 
						  mat3 Minv, 
						  vec3 light_pos, 
						  uint points, 
						  uint offset, 
						  bool two_sided) {
	// construct orthonormal basis around N
	vec3 T1, T2;
	T1 = normalize(V - N*dot(V, N));
	T2 = cross(N, T1);

	// rotate area light in (T1, T2, R) basis
	Minv = Minv * transpose(mat3(T1, T2, N));

	uint t=0;
	vec3 start = Minv * (light_pos + ltc_point(ltc_points[offset]) - P);
	vec3 p0,p1,p2;
	if (start.z < .0f) {
		vec3 next = Minv * (light_pos + ltc_point(ltc_points[offset + 1]) - P);
		for (t=0; t<points-1 && next.z<.0f; ++t) {
			start = next;
			next = Minv * (light_pos + ltc_point(ltc_points[offset + (t + 2) % points]) - P);
		}
		if (next.z < .0f)
			return vec3(0);

		start = -start.z * next + next.z * start;
		start.z = .0f;
		p1 = next;
	}
	else
		p1 = Minv * (light_pos + ltc_point(ltc_points[offset + 1]) - P);
	
	start = normalize(start);

	p0 = start;
	p1 = normalize(p1);
	p2 = normalize(Minv * (light_pos + ltc_point(ltc_points[offset + (t + 2) % points]) - P));

	// Integrate
	float sum = .0f;
	for (; t<points; ++t) {
		if (p0.z >= 0 && p1.z >= 0) {
			sum += ltc_integrate_edge(p0, p1);
		}
		else if (p1.z >= 0) {
			// Clip p0
			vec3 v0 = normalize(-p0.z * p1 + p1.z * p0);
			v0.z = .0f;
			sum += ltc_integrate_edge(v0, p1);
		}
		else {
			// Clip p1
			vec3 v1 = normalize(-p1.z * p0 + p0.z * p1);
			v1.z = .0f;
			sum += ltc_integrate_edge(p0, v1);

			if (p2.z >= 0) {
				vec3 v2 = normalize(-p1.z * p2 + p2.z * p1);
				v2.z = .0f;
				sum += ltc_integrate_edge(v1, v2);

				p1 = v2;
			}
			else {
				p1 = v1;
			}
		}

		uint next_idx = t + 3;
		p0 = p1;
		p1 = p2;
		p2 = next_idx < points ? normalize(Minv * (light_pos + ltc_point(ltc_points[offset + next_idx]) - P)) : start;
	}
	
	sum = two_sided ? 
			abs(sum) : 
			max(.0f, -sum);

	return vec3(sum) / two_pi;
}

/*
 *	Integrates spherical clamped cosine distribution of a projected convex polyhedron cross section.
 *	
 *	The input is a convex polyhedron, constructed from triangles. The input is clipped to the upper hemisphere of point P with normal N.
 *	Input is read from storage buffer ltc_points.
 *
 *	@param N			Normal, world coordinates.
 *	@param V			Position to eye vector, normalized.
 *	@param P			Position, world coordinates.
 *	@param Minv			Inverse of M, linear transformation matrix from clamped cosine distribution.
 *	@param light_pos	Light position, world coordinates.
 *	@param primitives	Primitives (triangles) count.
 *	@param offset		Offset into point buffer (ltc_points).
 */
vec3 ltc_evaluate_convex_polyhedron(vec3 N, 
									vec3 V, 
									vec3 P, 
									mat3 Minv, 
									vec3 light_pos, 
									uint primitives, 
									uint offset) {
	// construct orthonormal basis around N
	vec3 T1, T2;
	T1 = normalize(V - N*dot(V, N));
	T2 = cross(N, T1);

	// rotate area light in (T1, T2, R) basis
	Minv = Minv * transpose(mat3(T1, T2, N));

	vec3 accum = vec3(.0f);
	for (uint t=0; t<primitives; ++t) {
		// polygon (allocate 4 vertices for clipping)
		vec3 v[4];
		v[0] = Minv * (light_pos + ltc_point(ltc_points[offset + 3*t + 0]) - P);
		v[1] = Minv * (light_pos + ltc_point(ltc_points[offset + 3*t + 1]) - P);
		v[2] = Minv * (light_pos + ltc_point(ltc_points[offset + 3*t + 2]) - P);
	
		vec3 zs = vec3(v[0].z, v[1].z, v[2].z);
		bool all_below_horizon = all(lessThanEqual(zs, vec3(.0f)));
		bool all_above_horizon = all(greaterThan(zs, vec3(.0f)));

		int n;
		if (all_below_horizon)
			continue;
		else if (all_above_horizon)
			n = 3;
		else
			ltc_clip_triangle(v, n);

		// project onto sphere
		v[0] = normalize(v[0]);
		v[1] = normalize(v[1]);
		v[2] = normalize(v[2]);
		v[3] = normalize(v[3]);

		// integrate
		float sum = 0.0;

		sum += ltc_integrate_edge(v[0], v[1]);
		sum += ltc_integrate_edge(v[1], v[2]);
		sum += ltc_integrate_edge(v[2], v[3]);
		if (n == 4)
			sum += ltc_integrate_edge(v[3], v[0]);

		accum += max(.0f, -sum);
	}

	return vec3(accum) / two_pi;
}

/*
 *	Integrates spherical clamped cosine distribution of a projected sphere cross section.
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
	const float points_per_solid_angle = 40.f;

	// construct orthonormal basis around N
	vec3 T1, T2;
	T1 = normalize(V - N*dot(V, N));
	T2 = cross(N, T1);

	// rotate area light in (T1, T2, R) basis
	Minv = Minv * transpose(mat3(T1, T2, N));

	float sum = .0f;
	vec3 l = light_pos - P;

	float d = length(l);
	if (d <= r)
		return vec3(1.f);

	// Compute projected sphere cross section
	float cs_d = d - r * r / d;
	float h = d - cs_d;
	float cs_r = sqrt(r*r - h*h);
	l = l / d;
	r = cs_r / cs_d;
	
	// Compute solid angle, and use it to select amount of circle integration points
	float solid_angle = clamp(r, epsilon, 2);
	float n = 3.5f + solid_angle * points_per_solid_angle;
	float two_pi_recp_n = two_pi / n;

	// As the polygon inscribed in the circle has lower area then the circle, compensate with larger radius
	// Area of a regular n-vertex polygon inscribed in a circle is: r^2*n*sin(2pi/n)/2
	float sphere_area = pi; // * r^2
	float polygon_area = .5f * n * sin(two_pi_recp_n); // * r^2
	float multiplier = max(1.f, sqrt(sphere_area / polygon_area));

	// Create basis vectors for the projected sphere cross section
	vec3 t = cross(vec3(1,0,0), l);
	if (dot(t,t) < 1e-5f)
		t = cross(vec3(0,0,1), l);
	t = normalize(t);
	vec3 b = normalize(cross(l, t));

	t *= r * multiplier;
	b *= r * multiplier;
	
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
