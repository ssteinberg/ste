
const float pi = 3.1415926535897932384626433832795;
const float pi_2 = pi * .5f;
const float epsilon = .0000001f;

float max_element(vec2 v) {
	return max(v.x, v.y);
}

float min_element(vec2 v) {
	return min(v.x, v.y);
}

float max_element(vec3 v) {
	return max(v.x, max(v.y, v.z));
}

float min_element(vec3 v) {
	return min(v.x, min(v.y, v.z));
}

float max_element(vec4 v) {
	return max(max(v.x, v.y), max(v.w, v.z));
}

float min_element(vec4 v) {
	return min(min(v.x, v.y), min(v.w, v.z));
}
