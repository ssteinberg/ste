
#include "atmospherics_descriptor.glsl"

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

