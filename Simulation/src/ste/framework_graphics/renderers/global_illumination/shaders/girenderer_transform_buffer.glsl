
#include "project.glsl"
#include "dual_quaternion.glsl"

struct view_transform_buffer_struct {
	dual_quaternion view_transform;
	dual_quaternion inverse_view_transform;
};

struct proj_transform_buffer_struct {
	vec4 proj_xywz;
	uvec2 backbuffer_size;
	float tan_half_fovy;
	float aspect;
};

layout(std430, binding = 20) restrict readonly buffer view_transform_buffer_data {
	view_transform_buffer_struct view_transform_buffer;
};

layout(std430, binding = 21) restrict readonly buffer proj_transform_buffer_data {
	proj_transform_buffer_struct proj_transform_buffer;
};

vec3 transform_view(vec3 v) {
	return dquat_mul_vec(view_transform_buffer.view_transform, v);
}

vec3 transform_direction_view(vec3 dir) {
	return quat_mul_vec(view_transform_buffer.view_transform.real, dir);
}

vec3 transform_view_to_world_space(vec3 p) {
	return dquat_mul_vec(view_transform_buffer.inverse_view_transform, p);
}

vec3 transform_direction_view_to_world_space(vec3 dir) {
	return quat_mul_vec(view_transform_buffer.inverse_view_transform.real, dir);
}

vec4 project(vec4 v) {
	return project(proj_transform_buffer.proj_xywz, v);
}

vec4 project(vec3 v) {
	return project(vec4(v, 1));
}

vec3 unproject_screen_position(float depth, vec2 norm_frag_coords) {
	return unproject_screen_position(depth, norm_frag_coords, proj_transform_buffer.proj_xywz);
}

vec3 unproject_screen_position_with_z(float z, vec2 norm_frag_coords) {
	return unproject_screen_position_with_z(z, norm_frag_coords, proj_transform_buffer.proj_xywz);
}

float projection_near_clip() {
	return proj_transform_buffer.proj_xywz.z;
}

float projection_aspect() {
	return proj_transform_buffer.aspect;
}

float projection_tan_half_fovy() {
	return proj_transform_buffer.tan_half_fovy;
}

uvec2 backbuffer_size() {
	return proj_transform_buffer.backbuffer_size;
}

float project_depth(float z) {
	return project_depth(z, projection_near_clip());
}

float unproject_depth(float d) {
	return unproject_depth(d, projection_near_clip());
}
