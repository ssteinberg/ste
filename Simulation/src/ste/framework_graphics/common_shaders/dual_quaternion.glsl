
struct dual_quaternion {
	vec4 real, dual;
};

vec3 dquat_mul_vec(dual_quaternion q, vec3 v) {
	return v +
			2.f * cross(q.real.xyz, cross(q.real.xyz, v) + q.real.w * v) +
			2.f * (q.real.w * q.dual.xyz - q.dual.w * q.real.xyz + cross(q.real.xyz, q.dual.xyz));
}

