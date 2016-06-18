
const float pi = 3.1415926535897932384626433832795;
const float pi_2 = pi * .5f;
const float epsilon = .0000001f;

float max_element(vec3 v) {
	return max(v.x, max(v.y, v.z));
}

float min_element(vec3 v) {
	return min(v.x, min(v.y, v.z));
}
