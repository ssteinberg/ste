
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

vec2 OctWrap(vec2 v) {
    return (1.0 - abs(v.yx)) * vec2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0);
}

vec2 normal3x32_to_snorm2x32(vec3 n) {
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    n.xy = n.z >= 0.0 ? n.xy : OctWrap(n.xy);
    return n.xy;
}

vec3 snorm2x32_to_normal3x32(vec2 encN) {
    vec3 n;
    n.z = 1.0 - abs(encN.x) - abs(encN.y);
    n.xy = n.z >= 0.0 ? encN.xy : OctWrap(encN.xy);
    n = normalize(n);
    return n;
}
