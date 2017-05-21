
#include <quaternion.glsl>

struct dual_quaternion {
	vec4 real, dual;
};

dual_quaternion dquat_mul_dquat(dual_quaternion dq1, dual_quaternion dq2) {
	float mag = length(dq1.real);

	dual_quaternion dq;
	dq.real = quat_mul_quat(dq1.real, dq2.real);
	dq.dual = quat_mul_quat(dq1.real, dq2.dual) / mag + quat_mul_quat(dq1.dual, dq2.real);

	return dq;
}

vec3 dquat_mul_vec(dual_quaternion q, vec3 v) {
	return v +
			2.f * cross(q.real.xyz, cross(q.real.xyz, v) + q.real.w * v) +
			2.f * (q.real.w * q.dual.xyz - q.dual.w * q.real.xyz + cross(q.real.xyz, q.dual.xyz));
}
