
#include <constants.glsl>

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

float max(float x, float y, float z) {
	return max(max(x,y),z);
}

vec2 max(vec2 x, vec2 y, vec2 z) {
	return max(max(x,y),z);
}

vec3 max(vec3 x, vec3 y, vec3 z) {
	return max(max(x,y),z);
}

vec4 max(vec4 x, vec4 y, vec4 z) {
	return max(max(x,y),z);
}

float min(float x, float y, float z) {
	return min(min(x,y),z);
}

vec2 min(vec2 x, vec2 y, vec2 z) {
	return min(min(x,y),z);
}

vec3 min(vec3 x, vec3 y, vec3 z) {
	return min(min(x,y),z);
}

vec4 min(vec4 x, vec4 y, vec4 z) {
	return min(min(x,y),z);
}

float sqr(float x) {
	return x*x;
}

int sqr(int x) {
	return x*x;
}

uint sqr(uint x) {
	return x*x;
}

vec2 sqr(vec2 x) {
	return x*x;
}

vec3 sqr(vec3 x) {
	return x*x;
}

vec4 sqr(vec4 x) {
	return x*x;
}

/*
 *	@brief	Returns 1 if x>=0, -1 otherwise.
 */
float sign_ge_z(float x) {
	return step(0, x)*2.f - 1.f;
}

/*
 *	@brief	Returns 1 if x>=0, -1 otherwise.
 */
vec2 sign_ge_z(vec2 x) {
	return step(0, x)*2.f - 1.f;
}

/*
 *	@brief	Returns 1 if x>=0, -1 otherwise.
 */
vec3 sign_ge_z(vec3 x) {
	return step(0, x)*2.f - 1.f;
}

/*
 *	@brief	Returns 1 if x>=0, -1 otherwise.
 */
vec4 sign_ge_z(vec4 x) {
	return step(0, x)*2.f - 1.f;
}
