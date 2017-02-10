
#type compute
#version 450

#extension GL_ARB_bindless_texture : enable

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

void main() {
	int ll_id = int(gl_GlobalInvocationID.x) / 6;
	if (ll_id >= ll_counter)
		return;

	int face = int(gl_GlobalInvocationID.x) % 6;
	uint light_idx = ll[ll_id];
	light_descriptor ld = light_buffer[light_idx];
	
	if (light_type_is_directional(ld.type)) {
		directional_light(light_idx, face, ld);
	}
}
