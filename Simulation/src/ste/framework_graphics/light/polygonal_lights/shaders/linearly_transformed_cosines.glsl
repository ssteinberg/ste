
#include "common.glsl"

vec2 LTC_Coords(sampler2D texLSDMat, float cosTheta, float roughness)
{
	float theta = acos(cosTheta);
	vec2 coords = vec2(roughness, theta/(0.5*3.14159));

	vec2 size = textureSize(texLSDMat, 0);
	// scale and bias coordinates, for correct filtered lookup
	coords = coords*(size - vec2(1.f))/size + .5f/size;

	return coords;
}

mat3 LTC_Matrix(sampler2D texLSDMat, vec2 coord)
{
	// load inverse matrix
	vec4 t = texture(texLSDMat, coord);
	mat3 Minv = mat3(
		vec3(1,     0, t.y),
		vec3(  0, t.z,   0),
		vec3(t.w,   0, t.x)
	);

	return Minv;
}

float IntegrateEdge(vec3 v1, vec3 v2)
{
	float cosTheta = dot(v1, v2);
	cosTheta = clamp(cosTheta, -0.9999, 0.9999);

	float theta = acos(cosTheta);    
	float res = cross(v1, v2).z * theta / sin(theta);

	return res;
}

void ClipQuadToHorizon(inout vec3 L[5], out int n)
{
	// detect clipping config
	int config = 0;
	if (L[0].z > 0.0) config += 1;
	if (L[1].z > 0.0) config += 2;
	if (L[2].z > 0.0) config += 4;
	if (L[3].z > 0.0) config += 8;

	// clip
	n = 0;

	if (config == 0)
	{
		// clip all
	}
	else if (config == 1) // V1 clip V2 V3 V4
	{
		n = 3;
		L[1] = -L[1].z * L[0] + L[0].z * L[1];
		L[2] = -L[3].z * L[0] + L[0].z * L[3];
	}
	else if (config == 2) // V2 clip V1 V3 V4
	{
		n = 3;
		L[0] = -L[0].z * L[1] + L[1].z * L[0];
		L[2] = -L[2].z * L[1] + L[1].z * L[2];
	}
	else if (config == 3) // V1 V2 clip V3 V4
	{
		n = 4;
		L[2] = -L[2].z * L[1] + L[1].z * L[2];
		L[3] = -L[3].z * L[0] + L[0].z * L[3];
	}
	else if (config == 4) // V3 clip V1 V2 V4
	{
		n = 3;
		L[0] = -L[3].z * L[2] + L[2].z * L[3];
		L[1] = -L[1].z * L[2] + L[2].z * L[1];
	}
	else if (config == 5) // V1 V3 clip V2 V4) impossible
	{
		n = 0;
	}
	else if (config == 6) // V2 V3 clip V1 V4
	{
		n = 4;
		L[0] = -L[0].z * L[1] + L[1].z * L[0];
		L[3] = -L[3].z * L[2] + L[2].z * L[3];
	}
	else if (config == 7) // V1 V2 V3 clip V4
	{
		n = 5;
		L[4] = -L[3].z * L[0] + L[0].z * L[3];
		L[3] = -L[3].z * L[2] + L[2].z * L[3];
	}
	else if (config == 8) // V4 clip V1 V2 V3
	{
		n = 3;
		L[0] = -L[0].z * L[3] + L[3].z * L[0];
		L[1] = -L[2].z * L[3] + L[3].z * L[2];
		L[2] =  L[3];
	}
	else if (config == 9) // V1 V4 clip V2 V3
	{
		n = 4;
		L[1] = -L[1].z * L[0] + L[0].z * L[1];
		L[2] = -L[2].z * L[3] + L[3].z * L[2];
	}
	else if (config == 10) // V2 V4 clip V1 V3) impossible
	{
		n = 0;
	}
	else if (config == 11) // V1 V2 V4 clip V3
	{
		n = 5;
		L[4] = L[3];
		L[3] = -L[2].z * L[3] + L[3].z * L[2];
		L[2] = -L[2].z * L[1] + L[1].z * L[2];
	}
	else if (config == 12) // V3 V4 clip V1 V2
	{
		n = 4;
		L[1] = -L[1].z * L[2] + L[2].z * L[1];
		L[0] = -L[0].z * L[3] + L[3].z * L[0];
	}
	else if (config == 13) // V1 V3 V4 clip V2
	{
		n = 5;
		L[4] = L[3];
		L[3] = L[2];
		L[2] = -L[1].z * L[2] + L[2].z * L[1];
		L[1] = -L[1].z * L[0] + L[0].z * L[1];
	}
	else if (config == 14) // V2 V3 V4 clip V1
	{
		n = 5;
		L[4] = -L[0].z * L[3] + L[3].z * L[0];
		L[0] = -L[0].z * L[1] + L[1].z * L[0];
	}
	else if (config == 15) // V1 V2 V3 V4
	{
		n = 4;
	}
	
	if (n == 3)
		L[3] = L[0];
	if (n == 4)
		L[4] = L[0];
}

void ClipTriangleToHorizon(inout vec3 L[4], out int n)
{
	// detect clipping config
	int config = 0;
	if (L[0].z > 0.0) config += 1;
	if (L[1].z > 0.0) config += 2;
	if (L[2].z > 0.0) config += 4;

	// clip
	n = 0;

	if (config == 0)
	{
		// clip all
	}
	else if (config == 1) // V1 clip V2 V3
	{
		n = 3;
		L[1] = -L[1].z * L[0] + L[0].z * L[1];
		L[2] = -L[2].z * L[0] + L[0].z * L[2];
	}
	else if (config == 2) // V2 clip V1 V3
	{
		n = 3;
		L[0] = -L[0].z * L[1] + L[1].z * L[0];
		L[2] = -L[2].z * L[1] + L[1].z * L[2];
	}
	else if (config == 3) // V1 V2 clip V3
	{
		n = 4;
		L[3] = -L[2].z * L[0] + L[0].z * L[2];
		L[2] = -L[2].z * L[1] + L[1].z * L[2];
	}
	else if (config == 4) // V3 clip V1 V2
	{
		n = 3;
		L[0] = -L[0].z * L[2] + L[2].z * L[0];
		L[1] = -L[1].z * L[2] + L[2].z * L[1];
	}
	else if (config == 5) // V1 V3 clip V2
	{
		n = 4;
		L[3] = L[2];
		L[2] = -L[2].z * L[1] + L[1].z * L[2];
		L[1] = -L[1].z * L[0] + L[0].z * L[1];
	}
	else if (config == 6) // V2 V3 clip V1
	{
		n = 4;
		L[3] = -L[0].z * L[2] + L[2].z * L[0];
		L[0] = -L[0].z * L[1] + L[1].z * L[0];
	}
	else if (config == 7) // V1 V2 V3
	{
		n = 3;
	}
	
	if (n == 3)
		L[3] = L[0];
}

vec3 LTC_Evaluate(
	vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], bool twoSided)//, sampler2D texFilteredMap)
{
	// construct orthonormal basis around N
	vec3 T1, T2;
	T1 = normalize(V - N*dot(V, N));
	T2 = cross(N, T1);

	// rotate area light in (T1, T2, R) basis
	Minv = Minv * transpose(mat3(T1, T2, N));

	// polygon (allocate 5 vertices for clipping)
	vec3 L[5];
	L[0] = Minv * (points[0].xyz - P);
	L[1] = Minv * (points[1].xyz - P);
	L[2] = Minv * (points[2].xyz - P);
	L[3] = Minv * (points[3].xyz - P);
	L[4] = L[3]; // avoid warning

	vec3 textureLight = vec3(1, 1, 1);
#if LTC_TEXTURED
	//textureLight = FetchDiffuseFilteredTexture(texFilteredMap, L[0], L[1], L[2], L[3]);
#endif

	int n;
	ClipQuadToHorizon(L, n);
	
	if (n == 0)
		return vec3(0, 0, 0);

	// project onto sphere
	L[0] = normalize(L[0]);
	L[1] = normalize(L[1]);
	L[2] = normalize(L[2]);
	L[3] = normalize(L[3]);
	L[4] = normalize(L[4]);

	// integrate
	float sum = 0.0;

	sum += IntegrateEdge(L[0], L[1]);
	sum += IntegrateEdge(L[1], L[2]);
	sum += IntegrateEdge(L[2], L[3]);
	if (n >= 4)
		sum += IntegrateEdge(L[3], L[4]);
	if (n == 5)
		sum += IntegrateEdge(L[4], L[0]);

	// note: negated due to winding order
	sum = twoSided ? abs(sum) : max(0.0, -sum);

	vec3 Lo_i = vec3(sum, sum, sum);

	// scale by filtered light color
	Lo_i *= textureLight;

	return Lo_i / two_pi;
}

vec3 LTC_Evaluate(
	vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[9], bool twoSided)//, sampler2D texFilteredMap)
{
	// construct orthonormal basis around N
	vec3 T1, T2;
	T1 = normalize(V - N*dot(V, N));
	T2 = cross(N, T1);

	// rotate area light in (T1, T2, R) basis
	Minv = Minv * transpose(mat3(T1, T2, N));

	vec3 accum = vec3(.0f);
	for (int t=0; t<3; ++t) {
		// polygon (allocate 4 vertices for clipping)
		vec3 L[4];
		L[0] = Minv * (points[0 + t*3].xyz - P);
		L[1] = Minv * (points[1 + t*3].xyz - P);
		L[2] = Minv * (points[2 + t*3].xyz - P);
		L[3] = L[2]; // avoid warning

		vec3 textureLight = vec3(1, 1, 1);
	#if LTC_TEXTURED
		//textureLight = FetchDiffuseFilteredTexture(texFilteredMap, L[0], L[1], L[2], L[3]);
	#endif

		int n=3;
		//ClipTriangleToHorizon(L, n);
	
		if (n == 0)
			return vec3(0, 0, 0);

		// project onto sphere
		L[0] = normalize(L[0]);
		L[1] = normalize(L[1]);
		L[2] = normalize(L[2]);
		L[3] = normalize(L[3]);

		// integrate
		float sum = 0.0;

		sum += IntegrateEdge(L[0], L[1]);
		sum += IntegrateEdge(L[1], L[2]);
		sum += IntegrateEdge(L[2], L[3]);
		if (n == 4)
			sum += IntegrateEdge(L[3], L[0]);

		// note: negated due to winding order
		sum = twoSided ? abs(sum) : max(0.0, -sum);

		// scale by filtered light color
		accum += sum * textureLight;
	}

	vec3 Lo_i = vec3(accum);

	return Lo_i / two_pi;
}

vec3 LTC_Evaluate(
	vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 L, float r)
{
	const float points_per_solid_angle = 50.f;

	// construct orthonormal basis around N
	vec3 T1, T2;
	T1 = normalize(V - N*dot(V, N));
	T2 = cross(N, T1);

	// rotate area light in (T1, T2, R) basis
	Minv = Minv * transpose(mat3(T1, T2, N));

	float sum = .0f;
	vec3 l = L - P;

	vec3 t = cross(vec3(1,0,0), l);
	if (dot(t,t) < 1e-5f)
		t = cross(vec3(0,0,1), l);
	t = r * normalize(t);
	vec3 b = r * normalize(cross(l, t));
	
	float solid_angle = clamp(r / length(l), epsilon, 2);
	float total_points = 2.f + solid_angle * points_per_solid_angle;
	
	vec3 start_point = Minv * (l + t);
	start_point.z = max(.0f, start_point.z);
	start_point = normalize(start_point);

	vec3 p0 = start_point;
	for (float i=1; i<total_points; ++i) {
		float x = two_pi * i / total_points;

		vec3 p1 = l + cos(x) * t + sin(x) * b;
		p1 = Minv * p1;
		p1.z = max(.0f, p1.z);
		p1 = normalize(p1);
		
		sum += IntegrateEdge(p0, p1);

		p0 = p1;
	}
	
	sum += IntegrateEdge(p0, start_point);

	return abs(sum).xxx;
}
