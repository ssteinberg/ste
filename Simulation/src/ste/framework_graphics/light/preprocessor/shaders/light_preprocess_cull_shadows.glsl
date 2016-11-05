
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

const vec3 face_directions[6] = { vec3( 1, 0, 0),
								  vec3(-1, 0, 0),
								  vec3( 0, 1, 0),
								  vec3( 0,-1, 0),
								  vec3( 0, 0, 1),
								  vec3( 0, 0,-1) };
uniform float shadow_proj_near = 20.f;

void main() {
	int ll_id = int(gl_GlobalInvocationID.x) / 6;
	if (ll_id >= ll_counter)
		return;

	int face = int(gl_GlobalInvocationID.x) % 6;
	uint16_t light_idx = ll[ll_id];
	light_descriptor ld = light_buffer[light_idx];
	
	if (ld.type == LightTypeDirectional) {
		// Calculate cascades' projection matrices
		int cascade = face;
		if (cascade >= directional_light_cascades)
			return;
			
		vec3 l = ld.transformed_position;

		uint32_t cascade_idx = light_get_cascade_descriptor_idx(ld);
		light_cascade_descriptor cascade_descriptor = directional_lights_cascades[cascade_idx];

		// Calculate eye frustum portion for this cascade
		float near = light_cascade_near(cascade_descriptor, cascade);
		float far = light_cascade_far(cascade_descriptor, cascade);
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
	else {
		vec3 dir = quat_mul_vec(view_transform_buffer.view_transform.real, face_directions[face]);
		vec3 origin = ld.transformed_position;

		// Compute minimal bounding sphere of the shadow projection frustum
		float n = shadow_proj_near;
		float f = max(ld.effective_range, n);
		float h = f - n;

		float f2 = f*f;
		float a = .5f * (h + (n*n - f2) / h);

		vec3 c = origin + dir * (f - a);
		float r = sqrt(f2 + a*a);

		// If the shadow face intersects the view frustum, update the mask with the face id
		if (collision_sphere_infinite_frustum(c, r,
											  np, rp, lp, tp, bp)) {
			int mask = 1 << face;
			atomicAdd(light_buffer[light_idx].shadow_face_mask, mask);
		}
	}
}
