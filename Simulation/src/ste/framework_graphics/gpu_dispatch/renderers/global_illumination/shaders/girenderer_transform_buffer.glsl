
#include "project.glsl"
#include "dual_quaternion.glsl"

struct view_transform_buffer_struct {
	dual_quaternion view_transform;
	dual_quaternion inverse_view_transform;
};

struct proj_transform_buffer_struct {
	vec4 proj_xywz;
	uvec2 backbuffer_size;
};

layout(std430, binding = 20) restrict readonly buffer view_transform_buffer_data {
	view_transform_buffer_struct view_transform_buffer;
};

layout(std430, binding = 21) restrict readonly buffer proj_transform_buffer_data {
	proj_transform_buffer_struct proj_transform_buffer;
};

vec4 project(vec4 v) {
	return project(proj_transform_buffer.proj_xywz, v);
}

vec4 project(vec3 v) {
	return project(vec4(v, 1));
}

vec3 unproject_screen_position(float depth, vec2 norm_frag_coords) {
	return unproject_screen_position(depth, norm_frag_coords, proj_transform_buffer.proj_xywz);
}

float projection_near_clip() {
	return proj_transform_buffer.proj_xywz.z;
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
