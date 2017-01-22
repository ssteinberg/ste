
vec3 quat_mul_vec(vec4 q, vec3 v) {
	vec3 t = 2.f * cross(q.xyz, v);
	return v + q.w * t + cross(q.xyz, t);
}

vec4 quat_mul_quat(vec4 q1, vec4 q2) {
	vec4 q;
	q.w = q1.w * q2.w - dot(q1.xyz, q2.xyz);
	q.xyz = q1.w * q2.xyz + q2.w * q1.xyz + cross(q1.xyz, q2.xyz);

	return q;
}
