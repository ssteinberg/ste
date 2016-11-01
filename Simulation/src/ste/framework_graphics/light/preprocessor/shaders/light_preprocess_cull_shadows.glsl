
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 128) in;

#include "girenderer_transform_buffer.glsl"

#include "intersection.glsl"
#include "quaternion.glsl"
#include "transform.glsl"

#include "light.glsl"

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
			
		float dist = ld.directional_distance;
		vec3 l = ld.transformed_position;

		uint32_t cascade_idx = light_get_cascade_descriptor_idx(ld);
		light_cascade_descriptor cascade_descriptor = directional_lights_cascades[cascade_idx];
		
		vec3 X = cascade_descriptor.X.xyz;
		vec3 Y = cascade_descriptor.Y.xyz;

		// Calculate eye frutsum portion for this cascade
		float near = light_cascade_near(cascade_descriptor, cascade);
		float far = light_cascade_far(cascade_descriptor, cascade);
		vec2 t = projection_tan_half_fovy() * vec2(near, far);
		vec2 r = t * projection_aspect();

		vec4 frustum[8] = {
			vec4( r.x,  t.x, -near, 1),
			vec4(-r.x,  t.x, -near, 1),
			vec4( r.x, -t.x, -near, 1),
			vec4(-r.x, -t.x, -near, 1),
			vec4( r.y,  t.y, -far,  1),
			vec4(-r.y,  t.y, -far,  1),
			vec4( r.y, -t.y, -far,  1),
			vec4(-r.y, -t.y, -far,  1)
		};

		// Create the transform to cascade space matrix
		vec3 center = vec3(0, 0, -mix(near,far,.5f));
		vec3 lpos = center - dist*l;
		mat4 M = look_at(lpos, center, Y, l, X);

		// Calculate cascade viewport limits
		vec3 viewport_and_zcutoff = vec3(.0f);
		for (int i=0; i<8; ++i) {
			vec4 transformed = M * frustum[i];
			vec3 t = vec3(abs(transformed.xy), -transformed.z);
			viewport_and_zcutoff = max(viewport_and_zcutoff, t);
		}
		
		vec2 vp = viewport_and_zcutoff.xy;
		float z_cutoff = -viewport_and_zcutoff.z;

		float reserve = 32.f / 1024.0f;
		vp *= 1.f + reserve;
		vec2 recp_vp = 1.f / vp;
		
		M[0].xy *= recp_vp;
		M[1].xy *= recp_vp;
		M[2].xy *= recp_vp;
		M[3].xy *= recp_vp;
		mat4 cascade_proj = M;
		
		// Write viewport size and transform matrix to cascade data
		directional_lights_cascades[cascade_idx].cascade[cascade].recp_viewport_size = recp_vp;
		directional_lights_cascades[cascade_idx].cascade[cascade].z_cutoff = z_cutoff;
		directional_lights_cascades[cascade_idx].cascade[cascade].cascade_mat = cascade_proj;
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
