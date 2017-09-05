
#include <common.glsl>

struct ltc_element {
	uvec2 data;
};

layout(std430, set=2, binding=11) restrict readonly buffer shaped_lights_points_binding {
	ltc_element ltc_points[];
};


vec3 ltc_point(uint ltc_element_index) {
	ltc_element e = ltc_points[ltc_element_index];
	vec3 p = vec3(unpackHalf2x16(e.data.x), unpackHalf2x16(e.data.y).x);
	return p;
}

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
	else if (config == 14) { // V2 V3 V4 clip V1
		n = 5;
		L[4] = -L[0].z * L[3] + L[3].z * L[0];
		L[0] = -L[0].z * L[1] + L[1].z * L[0];
	}
	else if (config == 5 || config == 10)
		n = 0;
	
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
