
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

int max_element(ivec2 v) {
	return max(v.x, v.y);
}

int min_element(ivec2 v) {
	return min(v.x, v.y);
}

int max_element(ivec3 v) {
	return max(v.x, max(v.y, v.z));
}

int min_element(ivec3 v) {
	return min(v.x, min(v.y, v.z));
}

int max_element(ivec4 v) {
	return max(max(v.x, v.y), max(v.w, v.z));
}

int min_element(ivec4 v) {
	return min(min(v.x, v.y), min(v.w, v.z));
}

uint max_element(uvec2 v) {
	return max(v.x, v.y);
}

uint min_element(uvec2 v) {
	return min(v.x, v.y);
}

uint max_element(uvec3 v) {
	return max(v.x, max(v.y, v.z));
}

uint min_element(uvec3 v) {
	return min(v.x, min(v.y, v.z));
}

uint max_element(uvec4 v) {
	return max(max(v.x, v.y), max(v.w, v.z));
}

uint min_element(uvec4 v) {
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

int max(int x, int y, int z) {
	return max(max(x,y),z);
}

ivec2 max(ivec2 x, ivec2 y, ivec2 z) {
	return max(max(x,y),z);
}

ivec3 max(ivec3 x, ivec3 y, ivec3 z) {
	return max(max(x,y),z);
}

ivec4 max(ivec4 x, ivec4 y, ivec4 z) {
	return max(max(x,y),z);
}

int min(int x, int y, int z) {
	return min(min(x,y),z);
}

ivec2 min(ivec2 x, ivec2 y, ivec2 z) {
	return min(min(x,y),z);
}

ivec3 min(ivec3 x, ivec3 y, ivec3 z) {
	return min(min(x,y),z);
}

ivec4 min(ivec4 x, ivec4 y, ivec4 z) {
	return min(min(x,y),z);
}

uint max(uint x, uint y, uint z) {
	return max(max(x,y),z);
}

uvec2 max(uvec2 x, uvec2 y, uvec2 z) {
	return max(max(x,y),z);
}

uvec3 max(uvec3 x, uvec3 y, uvec3 z) {
	return max(max(x,y),z);
}

uvec4 max(uvec4 x, uvec4 y, uvec4 z) {
	return max(max(x,y),z);
}

uint min(uint x, uint y, uint z) {
	return min(min(x,y),z);
}

uvec2 min(uvec2 x, uvec2 y, uvec2 z) {
	return min(min(x,y),z);
}

uvec3 min(uvec3 x, uvec3 y, uvec3 z) {
	return min(min(x,y),z);
}

uvec4 min(uvec4 x, uvec4 y, uvec4 z) {
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

ivec2 sqr(ivec2 x) {
	return x*x;
}

ivec3 sqr(ivec3 x) {
	return x*x;
}

ivec4 sqr(ivec4 x) {
	return x*x;
}

uvec2 sqr(uvec2 x) {
	return x*x;
}

uvec3 sqr(uvec3 x) {
	return x*x;
}

uvec4 sqr(uvec4 x) {
	return x*x;
}

/*
 *	@brief	Returns 1 if x>=0, -1 otherwise.
 */
float sign_ge_z(float x) {
	return step(.0f, x)*2.f - 1.f;
}

/*
 *	@brief	Returns 1 if x>=0, -1 otherwise.
 */
vec2 sign_ge_z(vec2 x) {
	return step(.0f, x)*2.f - 1.f;
}

/*
 *	@brief	Returns 1 if x>=0, -1 otherwise.
 */
vec3 sign_ge_z(vec3 x) {
	return step(.0f, x)*2.f - 1.f;
}

/*
 *	@brief	Returns 1 if x>=0, -1 otherwise.
 */
vec4 sign_ge_z(vec4 x) {
	return step(.0f, x)*2.f - 1.f;
}
