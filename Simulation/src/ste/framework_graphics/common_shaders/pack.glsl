
uvec3 snorm12x2_to_unorm8x3(vec2 f) {
	vec2 u = vec2(round(clamp(f, -1.f, 1.f) * 2047.f + 2047.f));
	float t = floor(u.y / 256.f);

	vec3 v = floor(vec3(u.x / 16.f,
						fract(u.x / 16.f) * 256.f + t,
						u.y - t * 256.f));
	return uvec3(clamp(ivec3(v), ivec3(0), ivec3(255)));
}

vec2 unorm8x3_to_snorm12x2(uvec3 iu) {
	vec3 u = vec3(iu);

	u.y *= (1.f / 16.f);
	vec2 s = vec2(u.x * 16.f + floor(u.y),
				  fract(u.y) * (16.f * 256.f) + u.z);

	return clamp(s * (1.f / 2047.f) - 1.f, vec2(-1.f), vec2(1.f));
}

vec2 signNotZero(vec2 v) {
	return vec2((v.x >= 0.f) ? +1.f : -1.f, (v.y >= 0.f) ? +1.f : -1.f);
}

// Assume normalized input. Output is on [-1, 1] for each component.
vec2 float32x3_to_oct(in vec3 v) {
	// Project the sphere onto the octahedron, and then onto the xy plane
	vec2 p = v.xy * (1.f / (abs(v.x) + abs(v.y) + abs(v.z)));
	// Reflect the folds of the lower hemisphere over the diagonals
	return (v.z <= 0.f) ? ((1.f - abs(p.yx)) * signNotZero(p)) : p;
}

vec3 oct_to_float32x3(vec2 e) {
	vec3 v = vec3(e.xy, 1.f - abs(e.x) - abs(e.y));
	if (v.z < 0) v.xy = (1.f - abs(v.yx)) * signNotZero(v.xy);
	return normalize(v);
}

vec2 float32x3_to_octn_precise(vec3 v, const in int n) {
	// http://jcgt.org/published/0003/02/01/paper.pdf

	vec2 s = float32x3_to_oct(v); // Remap to the square
	// Each snorm’s max value interpreted as an integer,
	// e.g., 127.0 for snorm8
	float M = float(1 << ((n/2) - 1)) - 1.f;
	// Remap components to snorm(n/2) precision...with floor instead
	// of round (see equation 1)
	s = floor(clamp(s, -1.f, +1.f) * M) * (1.f / M);
	vec2 bestRepresentation = s;
	float highestCosine = dot(oct_to_float32x3(s), v);
	// Test all combinations of floor and ceil and keep the best.
	// Note that at +/- 1, this will exit the square... but that
	// will be a worse encoding and never win.
	for (int i = 0; i <= 1; ++i)
		for (int j = 0; j <= 1; ++j)
			// This branch will be evaluated at compile time
			if ((i != 0) || (j != 0)) {
				// Offset the bit pattern (which is stored in floating
				// point!) to effectively change the rounding mode
				// (when i or j is 0: floor, when it is one: ceiling)
				vec2 candidate = vec2(i, j) * (1 / M) + s;
				float cosine = dot(oct_to_float32x3(candidate), v);
				if (cosine > highestCosine) {
					bestRepresentation = candidate;
					highestCosine = cosine;
				}
			}
	return bestRepresentation;
}
