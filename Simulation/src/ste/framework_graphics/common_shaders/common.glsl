
#include "constants.glsl"

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

float max3(float x, float y, float z) {
	return max(max(x,y),z);
}

vec2 max3(vec2 x, vec2 y, vec2 z) {
	return max(max(x,y),z);
}

vec3 max3(vec3 x, vec3 y, vec3 z) {
	return max(max(x,y),z);
}

vec4 max3(vec4 x, vec4 y, vec4 z) {
	return max(max(x,y),z);
}

float min3(float x, float y, float z) {
	return min(min(x,y),z);
}

vec2 min3(vec2 x, vec2 y, vec2 z) {
	return min(min(x,y),z);
}

vec3 min3(vec3 x, vec3 y, vec3 z) {
	return min(min(x,y),z);
}

vec4 min3(vec4 x, vec4 y, vec4 z) {
	return min(min(x,y),z);
}
