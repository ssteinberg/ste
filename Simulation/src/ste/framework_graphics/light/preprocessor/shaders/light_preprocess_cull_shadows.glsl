
#type compute
#version 450

layout(local_size_x = 128) in;

#include "girenderer_transform_buffer.glsl"

#include "intersection.glsl"
#include "quaternion.glsl"

#include "shadow_common.glsl"
#include "light.glsl"
#include "light_cascades.glsl"

layout(std430, binding = 2) restrict buffer light_data {
	light_descriptor light_buffer[];
};

layout(shared, binding = 4) restrict readonly buffer ll_counter_data {
	uint ll_counter;
};
layout(shared, binding = 5) restrict readonly buffer ll_data {
	uint ll[];
};
layout(shared, binding = 6) restrict buffer directional_lights_cascades_data {
	light_cascade_descriptor directional_lights_cascades[];
};

uniform vec4 np, rp, lp, tp, bp;
uniform float cascades_depths[directional_light_cascades];

const vec3 face_directions[6] = { vec3( 1, 0, 0),
								  vec3(-1, 0, 0),
								  vec3( 0, 1, 0),
								  vec3( 0,-1, 0),
								  vec3( 0, 0, 1),
								  vec3( 0, 0,-1) };

void directional_light(uint light_idx, int cascade, light_descriptor ld) {
	if (cascade >= directional_light_cascades)
		return;
			
	vec3 l = ld.transformed_position;

	uint cascade_idx = light_get_cascade_descriptor_idx(ld);
	light_cascade_descriptor cascade_descriptor = directional_lights_cascades[cascade_idx];

	// Calculate eye frustum portion for this cascade
	float near = light_cascade_near(cascade, cascades_depths);
	float far = light_cascade_far(cascade, cascades_depths);
	vec2 top = projection_tan_half_fovy() * vec2(near, far);
	vec2 right = top * projection_aspect();

	// Create the temporary, untranslated, unprojected transform to cascade space matrix,
	// and use it to calculate the corrected eye distance and viewport size
	mat3x4 M = light_cascade_projection(cascade_descriptor,
										cascade,
										l, .0f, vec2(1.f),
										near, far);

	// Frustum vertices
	vec4 frustum[8] = {
		vec4( right.x,  top.x, -near, 1),
		vec4(-right.x,  top.x, -near, 1),
		vec4( right.x, -top.x, -near, 1),
		vec4(-right.x, -top.x, -near, 1),
		vec4( right.y,  top.y, -far,  1),
		vec4(-right.y,  top.y, -far,  1),
		vec4( right.y, -top.y, -far,  1),
		vec4(-right.y, -top.y, -far,  1)
	};

	// Calculate cascade viewport limits
	vec4 t = vec4(0, 0, -inf, -inf);
	for (int i=0; i<8; ++i) {
		vec3 transformed = frustum[i] * M;
		t = max(t, vec4(abs(transformed.xy), transformed.z, -transformed.z));
	}
		
	// Viewport size
	vec2 vp = t.xy * cascade_viewport_reserve;
	vec2 recp_vp = 1.f / vp;

	float eye_dist = t.z + cascade_projection_eye_distance;
	float far_clip = t.w + eye_dist;
		
	// Write cascade data and z limits
	directional_lights_cascades[cascade_idx].cascades_data[cascade] = vec4(recp_vp, eye_dist, far_clip);
}

void spherical_light(uint light_idx, int face, light_descriptor ld) {
	// Calculate shadow projection face frustum
	vec3 origin = ld.transformed_position;
	vec3 dir = face_directions[face];

	float n = ld.radius;
	float f = max(ld.effective_range, n);

	vec3 t = vec3(1,-1, 0);
	vec3 u[4];

	if ((face & 4) != 0) {
		u[0] = dir + t.xxz;
		u[1] = dir + t.xyz;
		u[2] = dir + t.yxz;
		u[3] = dir + t.yyz;
	}
	else if ((face & 2) != 0) {
		u[0] = dir + t.xzx;
		u[1] = dir + t.xzy;
		u[2] = dir + t.yzx;
		u[3] = dir + t.yzy;
	}
	else {
		u[0] = dir + t.zxx;
		u[1] = dir + t.zxy;
		u[2] = dir + t.zyx;
		u[3] = dir + t.zyy;
	}

	// Check frustum-frustum intersection
	bvec4 bn0, bn1;
	bvec4 br0, br1;
	bvec4 bl0, bl1;
	bvec4 bt0, bt1;
	bvec4 bb0, bb1;
	for (int i=0; i<4; ++i) {
		vec3 w = transform_direction_view(u[i]);
		vec3 v0 = origin + w * n;
		vec3 v1 = origin + w * f;
			
		bn0[i] = dot(np.xyz, v0) + np.w <= 0;
		br0[i] = dot(rp.xyz, v0) + rp.w <= 0;
		bl0[i] = dot(lp.xyz, v0) + lp.w <= 0;
		bt0[i] = dot(tp.xyz, v0) + tp.w <= 0;
		bb0[i] = dot(bp.xyz, v0) + bp.w <= 0;

		bn1[i] = dot(np.xyz, v1) + np.w <= 0;
		br1[i] = dot(rp.xyz, v1) + rp.w <= 0;
		bl1[i] = dot(lp.xyz, v1) + lp.w <= 0;
		bt1[i] = dot(tp.xyz, v1) + tp.w <= 0;
		bb1[i] = dot(bp.xyz, v1) + bp.w <= 0;
	}
		
	bool bn = all(bn0) && all(bn1);
	bool br = all(br0) && all(br1);
	bool bl = all(bl0) && all(bl1);
	bool bt = all(bt0) && all(bt1);
	bool bb = all(bb0) && all(bb1);

	// If exists a plane such that all face frustum vertices are behind it, reject
	if (bn || br || bl || bt || bb)
		return;

	int mask = 1 << face;
	atomicAdd(light_buffer[light_idx].shadow_face_mask, mask);
}

void main() {
	int ll_id = int(gl_GlobalInvocationID.x) / 6;
	if (ll_id >= ll_counter)
		return;

	int face = int(gl_GlobalInvocationID.x) % 6;
	uint light_idx = ll[ll_id];
	light_descriptor ld = light_buffer[light_idx];
	
	if (ld.type == LightTypeDirectional) {
		directional_light(light_idx, face, ld);
	}
	else {
		spherical_light(light_idx, face, ld);
	}
}
