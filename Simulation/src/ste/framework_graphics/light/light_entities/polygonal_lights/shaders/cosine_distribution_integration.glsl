
#include <common.glsl>
#include <linearly_transformed_cosines.glsl>

/*
 *	Integrates spherical cosine distribution of a projected quad.
 *	
 *	The input is a quad, which is read from storage buffer ltc_points.
 *
 *	@param P			Position, world coordinates.
 *	@param L			Position to light source, normalized.
 *	@param light_pos	Light position, world coordinates.
 *	@param offset		Offset into point buffer (ltc_points).
 *	@param two_sided	Single or two-sided shape. Must be false for polyhedrons.
 */
vec3 integrate_cosine_distribution_quad(vec3 P, 
										vec3 L,
										vec3 light_pos, 
										uint offset, 
										bool two_sided) {//, sampler2D texFilteredMap) 
	// construct orthonormal basis around L
	vec3 T1 = vec3(1,0,0);
	if (dot(T1, L) > 0.99)
		T1 = vec3(0,1,0);
	vec3 T2 = normalize(cross(T1, L));
	T1 = cross(L, T2);

	mat3 M = transpose(mat3(T1, T2, L));

	vec3 v[4];
	v[0] = M * (light_pos + ltc_point(offset + 0) - P);
	v[1] = M * (light_pos + ltc_point(offset + 1) - P);
	v[2] = M * (light_pos + ltc_point(offset + 2) - P);
	v[3] = M * (light_pos + ltc_point(offset + 3) - P);

	//vec3 textureLight = FetchDiffuseFilteredTexture(texFilteredMap, v[0], v[1], v[2], v[3]);

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
	sum += ltc_integrate_edge(v[3], v[0]);

	sum = two_sided ? 
			abs(sum) : 
			max(.0f, -sum);

	return vec3(sum) / two_pi;
}

/*
 *	Integrates spherical cosine distribution of a projected polygon cross section.
 *	
 *	The input is an arbitary (planar) polygon. 
 *	Input is read from storage buffer ltc_points.
 *
 *	@param P			Position, world coordinates.
 *	@param L			Position to light source, normalized.
 *	@param light_pos	Light position, world coordinates.
 *	@param points		Point count. Must be >2.
 *	@param offset		Offset into point buffer (ltc_points).
 *	@param two_sided	Single or two-sided shape. Must be false for polyhedrons.
 */
vec3 integrate_cosine_distribution_polygon(vec3 P, 
										   vec3 L, 
										   vec3 light_pos, 
										   uint points, 
										   uint offset, 
										   bool two_sided) {
	// construct orthonormal basis around L
	vec3 T1 = vec3(1,0,0);
	if (dot(T1, L) > 0.99)
		T1 = vec3(0,1,0);
	vec3 T2 = normalize(cross(T1, L));
	T1 = cross(L, T2);

	// rotate area light in (T1, T2, R) basis
	mat3 M = transpose(mat3(T1, T2, L));

	uint t=0;
	vec3 start = M * (light_pos + ltc_point(offset) - P);
	vec3 p0,p1,p2;
	if (start.z < .0f) {
		vec3 next = M * (light_pos + ltc_point(offset + 1) - P);
		for (t=0; t<points-1 && next.z<.0f; ++t) {
			start = next;
			next = M * (light_pos + ltc_point(offset + (t + 2) % points) - P);
		}
		if (next.z < .0f)
			return vec3(0);

		start = -start.z * next + next.z * start;
		start.z = .0f;
		p1 = next;
	}
	else
		p1 = M * (light_pos + ltc_point(offset + 1) - P);
	
	start = normalize(start);

	p0 = start;
	p1 = normalize(p1);
	p2 = normalize(M * (light_pos + ltc_point(offset + (t + 2) % points) - P));

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
		p2 = next_idx < points ? normalize(M * (light_pos + ltc_point(offset + next_idx) - P)) : start;
	}
	
	sum = two_sided ? 
			abs(sum) : 
			max(.0f, -sum);

	return vec3(sum) / two_pi;
}

/*
 *	Integrates spherical clamped cosine distribution of a projected convex polyhedron cross section.
 *	
 *	The input is a convex polyhedron, constructed from triangles.
 *	Input is read from storage buffer ltc_points.
 *
 *	@param P			Position, world coordinates.
 *	@param L			Position to light source, normalized.
 *	@param light_pos	Light position, world coordinates.
 *	@param primitives	Primitives (triangles) count.
 *	@param offset		Offset into point buffer (ltc_points).
 */
vec3 integrate_cosine_distribution_convex_polyhedron(vec3 P, 
													 vec3 L, 
													 vec3 light_pos, 
													 uint primitives, 
													 uint offset) {
	// construct orthonormal basis around L
	vec3 T1 = vec3(1,0,0);
	if (dot(T1, L) > 0.99)
		T1 = vec3(0,1,0);
	vec3 T2 = normalize(cross(T1, L));
	T1 = cross(L, T2);

	mat3 M = transpose(mat3(T1, T2, L));

	vec3 accum = vec3(.0f);
	for (int t=0; t<primitives; ++t) {
		vec3 v[3];
		v[0] = M * (light_pos + ltc_point(offset + 3*t + 0) - P);
		v[1] = M * (light_pos + ltc_point(offset + 3*t + 1) - P);
		v[2] = M * (light_pos + ltc_point(offset + 3*t + 2) - P);
	
		vec3 zs = vec3(v[0].z, v[1].z, v[2].z);
		bool all_below_horizon = all(lessThanEqual(zs, vec3(.0f)));

		if (all_below_horizon)
			continue;

		// project onto sphere
		v[0] = normalize(v[0]);
		v[1] = normalize(v[1]);
		v[2] = normalize(v[2]);

		// integrate
		float sum = 0.0;

		sum += ltc_integrate_edge(v[0], v[1]);
		sum += ltc_integrate_edge(v[1], v[2]);
		sum += ltc_integrate_edge(v[2], v[0]);

		accum += max(.0f, -sum);
	}

	return vec3(accum) / two_pi;
}

/*
 *	Integrates spherical cosine distribution of a projected sphere cross section.
 *
 *	@param d	Distance to the sphere
 *	@param r	Radius of the sphere
 */
float integrate_cosine_distribution_sphere_cross_section(float d, float r) {
	if (d <= r)
		return 1.f;

	float r_over_d = r / d;
	float sin2_theta = sqr(r_over_d);

	return sin2_theta;
}
