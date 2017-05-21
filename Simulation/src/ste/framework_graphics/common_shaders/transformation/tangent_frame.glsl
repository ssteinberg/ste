
#include <quaternion.glsl>

mat2x3 extract_tangent_bitangent_from_tangent_frame(vec4 q) {
	return mat2x3(
		1.f - 2.f*(q.y*q.y + q.z*q.z),	2.f*(q.x*q.y + q.w*q.z),		2.f*(q.x*q.z - q.w*q.y),
		2.f*(q.x*q.y - q.w*q.z),		1.f - 2.f*(q.x*q.x + q.z*q.z),	2.f*(q.y*q.z + q.w*q.x)
	);
}

mat3 extract_tangent_frame(vec4 world_transform, vec4 tangent_frame) {
	vec4 q = quat_mul_quat(world_transform, tangent_frame);
	mat2x3 bt = extract_tangent_bitangent_from_tangent_frame(q);
	return mat3(
		bt[1],
		bt[0],
		cross(bt[0],bt[1]) * sign(tangent_frame.w)
	);
}
