
#include "common.glsl"
#include "linearly_transformed_cosines.glsl"

/*
 *	Integrates spherical cosine distribution of a projected quad.
 *	
 *	The input is a quad, which is then clipped to the upper hemisphere of point P with normal N.
 *	Input is read from storage buffer ltc_points.
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
	v[0] = M * (light_pos + ltc_points[offset + 0].xyz - P);
	v[1] = M * (light_pos + ltc_points[offset + 1].xyz - P);
	v[2] = M * (light_pos + ltc_points[offset + 2].xyz - P);
	v[3] = M * (light_pos + ltc_points[offset + 3].xyz - P);

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
 *	Integrates spherical clamped cosine distribution of a projected polygon or convex polyhedron cross section.
 *	
 *	The input is an arbitary polygon or a convex polyhedron, constructed from triangles. In the latter 
 *	case the light can not be two-sided. The input is clipped to the upper hemisphere of point P with normal N.
 *	Input is read from storage buffer ltc_points.
 *
 *	@param P			Position, world coordinates.
 *	@param L			Position to light source, normalized.
 *	@param light_pos	Light position, world coordinates.
 *	@param primitives	Primitives (triangles) count.
 *	@param offset		Offset into point buffer (ltc_points).
 *	@param two_sided	Single or two-sided shape. Must be false for polyhedrons.
 */
vec3 ltc_evaluate_polygon(vec3 P, 
						  vec3 L, 
						  vec3 light_pos, 
						  uint primitives, 
						  uint offset, 
						  bool two_sided) {//, sampler2D texFilteredMap)
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
		v[0] = M * (light_pos + ltc_points[offset + 3*t + 0].xyz - P);
		v[1] = M * (light_pos + ltc_points[offset + 3*t + 1].xyz - P);
		v[2] = M * (light_pos + ltc_points[offset + 3*t + 2].xyz - P);

		//vec3 textureLight = FetchDiffuseFilteredTexture(texFilteredMap, v[0], v[1], v[2], v[3]);
	
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

		// note: negated due to winding order
		sum = two_sided ? 
				abs(sum) : 
				max(.0f, -sum);

		accum += sum;
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

	float d_over_r = d / r;
	float sin2_theta = 1 / (sqr(d_over_r) - 1.f);

	return sin2_theta;
}
