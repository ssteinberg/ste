
#include <project.glsl>
#include <dual_quaternion.glsl>

struct view_transform_buffer_struct {
	dual_quaternion view_transform;
	dual_quaternion inverse_view_transform;
	vec4 eye_position;
};

struct proj_transform_buffer_struct {
	vec4 proj_xywz;
	uvec2 backbuffer_size;
	float tan_half_fovy;
	float aspect;
};

layout(std430, set=2, binding=0) restrict readonly buffer view_transform_buffer_binding {
	view_transform_buffer_struct view_transform_buffer;
};

layout(std430, set=2, binding=1) restrict readonly buffer proj_transform_buffer_binding {
	proj_transform_buffer_struct proj_transform_buffer;
};

/*
*	Returns the current eye (camera) position in world coordinates
*/
vec3 eye_position() {
	return view_transform_buffer.eye_position.xyz;
}

/*
*	Transforms a point v from world space to eye space
*/
vec3 transform_view(vec3 v) {
	return dquat_mul_vec(view_transform_buffer.view_transform, v);
}

/*
*	Transforms a direction v from world space to eye space
*/
vec3 transform_direction_view(vec3 dir) {
	return quat_mul_vec(view_transform_buffer.view_transform.real, dir);
}

/*
*	Transforms a point v from eye space back to world space
*/
vec3 transform_view_to_world_space(vec3 p) {
	return dquat_mul_vec(view_transform_buffer.inverse_view_transform, p);
}

/*
*	Transforms a direction v from eye space back to world space
*/
vec3 transform_direction_view_to_world_space(vec3 dir) {
	return quat_mul_vec(view_transform_buffer.inverse_view_transform.real, dir);
}

/*
*	Project a point v from eye space to homogeneous clip coordinates
*/
vec4 project(vec4 v) {
	return project(proj_transform_buffer.proj_xywz, v);
}

/*
*	Project a point v from eye space to homogeneous clip coordinates
*/
vec4 project(vec3 v) {
	return project(vec4(v, 1));
}

/*
*	Unprojects a screen position, given with depth value and normalized screen coordinates, into eye space
*/
vec3 unproject_screen_position(float depth, vec2 norm_frag_coords) {
	return unproject_screen_position(depth, norm_frag_coords, proj_transform_buffer.proj_xywz);
}

/*
*	Unprojects a screen position, given with a eye space z value and normalized screen coordinates, into eye space
*/
vec3 unproject_screen_position_with_z(float z, vec2 norm_frag_coords) {
	return unproject_screen_position_with_z(z, norm_frag_coords, proj_transform_buffer.proj_xywz);
}

/*
*	Returns the current configured near-clipping -plane distance of the view to clip projection
*/
float projection_near_clip() {
	return proj_transform_buffer.proj_xywz.z;
}

/*
*	Returns the current configured width-to-height aspect ratio of the view to clip projection
*/
float projection_aspect() {
	return proj_transform_buffer.aspect;
}

/*
*	Returns the current configured tangent of half of the vertical field-of-view of the view to clip projection
*/
float projection_tan_half_fovy() {
	return proj_transform_buffer.tan_half_fovy;
}

/*
*	Returns the dimensions of the back-buffer
*/
uvec2 backbuffer_size() {
	return proj_transform_buffer.backbuffer_size;
}

/*
*	Projects an eye space z value to a depth value
*/
float project_depth(float z) {
	return project_depth(z, projection_near_clip());
}

/*
*	Unprojects a depth value to an eye space z value
*/
float unproject_depth(float d) {
	return unproject_depth(d, projection_near_clip());
}
