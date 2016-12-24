
uint vec4ToUint(vec4 v) {
	v *= 255.f;
	uint r = uint(v.r) & 0x0FF;
	uint g = uint(v.g) & 0x0FF;
	uint b = uint(v.b) & 0x0FF;
	uint a = uint(v.a) & 0x0FF;
	return (r) | (g << 8U) | (b << 16U) | (a << 24U);
}

vec4 uintToVec4(uint u) {
	float r = float(u & 0xFF);
	float g = float((u & 0xFF00) >> 8U);
	float b = float((u & 0xFF0000) >> 16U);
	float a = float((u & 0xFF000000) >> 24U);
	return vec4(r,g,b,a) / 255.f;
}

uint vec3ToUint(vec3 v) {
	v *= 255.f;
	uint r = uint(v.r) & 0x0FF;
	uint g = uint(v.g) & 0x0FF;
	uint b = uint(v.b) & 0x0FF;
	return (r) | (g << 8U) | (b << 16U);
}

vec3 uintToVec3(uint u) {
	float r = float(u & 0xFF);
	float g = float((u & 0xFF00) >> 8U);
	float b = float((u & 0xFF0000) >> 16U);
	return vec3(r,g,b) / 255.f;
}
