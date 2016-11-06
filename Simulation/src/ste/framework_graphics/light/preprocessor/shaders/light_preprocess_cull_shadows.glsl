
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

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
	uint32_t ll_counter;
};
layout(shared, binding = 5) restrict readonly buffer ll_data {
	uint16_t ll[];
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

void directional_light(uint16_t light_idx, int cascade, light_descriptor ld) {
	if (cascade >= directional_light_cascades)
		return;
			
	vec3 l = ld.transformed_position;

	uint32_t cascade_idx = light_get_cascade_descriptor_idx(ld);
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
		t = max(t, vec4(abs(transformed.xy), -transformed.z, transformed.z));
	}
		
	// Viewport size
	vec2 vp = t.xy * cascade_viewport_reserve;
	vec2 recp_vp = 1.f / vp;

	vec2 z_limits = t.zw;
		
	// Write cascade data and z limits
	directional_lights_cascades[cascade_idx].cascades_data[cascade] = vec4(recp_vp, z_limits);
}

void spherical_light(uint16_t light_idx, int face, light_descriptor ld) {
	// Calculate shadow projection face frustum
	vec3 origin = ld.transformed_position;
	vec3 dir = face_directions[face];

	float n = ld.radius;
	float f = max(ld.effective_range, n);

	vec3 t = vec3(1,-1, 0);
	vec3 u0, u1, u2, u3;
	if (face == 0 || face == 1) {
		u0 = dir + t.zxx;
		u1 = dir + t.zxy;
		u2 = dir + t.zyx;
		u3 = dir + t.zyy;
	}
	else if (face == 2 || face == 3) {
		u0 = dir + t.xzx;
		u1 = dir + t.xzy;
		u2 = dir + t.yzx;
		u3 = dir + t.yzy;
	}
	else {
		u0 = dir + t.xxz;
		u1 = dir + t.xyz;
		u2 = dir + t.yxz;
		u3 = dir + t.yyz;
	}

	vec3 shadow_proj_frustum_vertices[8] = {
		u0 * n,
		u1 * n,
		u2 * n,
		u3 * n,
		u0 * f,
		u1 * f,
		u2 * f,
		u3 * f
	};

	// Check frustum-frustum intersection
	bvec4 bn0, bn1;
	bvec4 br0, br1;
	bvec4 bl0, bl1;
	bvec4 bt0, bt1;
	bvec4 bb0, bb1;
	for (int i=0; i<4; ++i) {
		vec3 v = origin + shadow_proj_frustum_vertices[i];
			
		bn0[i] = dot(np.xyz, v) + np.w <= 0;
		br0[i] = dot(rp.xyz, v) + rp.w <= 0;
		bl0[i] = dot(lp.xyz, v) + lp.w <= 0;
		bt0[i] = dot(tp.xyz, v) + tp.w <= 0;
		bb0[i] = dot(bp.xyz, v) + bp.w <= 0;
	}
	for (int i=0; i<4; ++i) {
		vec3 v = origin + shadow_proj_frustum_vertices[i+4];
			
		bn1[i] = dot(np.xyz, v) + np.w <= 0;
		br1[i] = dot(rp.xyz, v) + rp.w <= 0;
		bl1[i] = dot(lp.xyz, v) + lp.w <= 0;
		bt1[i] = dot(tp.xyz, v) + tp.w <= 0;
		bb1[i] = dot(bp.xyz, v) + bp.w <= 0;
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
	uint16_t light_idx = ll[ll_id];
	light_descriptor ld = light_buffer[light_idx];
	
	if (ld.type == LightTypeDirectional) {
		directional_light(light_idx, face, ld);
	}
	else {
		spherical_light(light_idx, face, ld);
	}
}
