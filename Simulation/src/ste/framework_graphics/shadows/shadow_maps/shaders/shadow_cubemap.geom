
#type geometry
#version 450
#extension GL_NV_gpu_shader5 : require

layout(triangles) in;
layout(triangle_strip, max_vertices=15) out;

#include "light.glsl"
#include "shadow_projection_instance_to_ll_idx_translation.glsl"

in vs_out {
	vec3 normal;
	vec2 uv;
	flat int matIdx;
	flat int instanceIdx;
	flat uint drawIdx;
} vin[];

out frag_in {
	vec2 uv;
	flat int matIdx;
} vout;

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};

layout(shared, binding = 4) restrict readonly buffer ll_counter_data {
	uint32_t ll_counter;
};
layout(shared, binding = 5) restrict readonly buffer ll_data {
	uint16_t ll[];
};

layout(std430, binding = 8) restrict readonly buffer shadow_projection_instance_to_ll_idx_translation_data {
	shadow_projection_instance_to_ll_idx_translation sproj_id_to_llid_tt[max_active_lights_per_frame];
};
layout(std430, binding = 9) restrict readonly buffer projection_data {
	mat4 shadow_transforms[];
};

uniform float far;

void process(int face, uint16_t l, vec4 vertices[3]) {
	vec4 transformed_vertices[3];

	for (int j = 0; j < 3; ++j)
		transformed_vertices[j] = shadow_transforms[face] * vertices[j];

	if (transformed_vertices[0].x > transformed_vertices[0].w &&
		transformed_vertices[1].x > transformed_vertices[1].w &&
		transformed_vertices[2].x > transformed_vertices[2].w)
		return;
	if (transformed_vertices[0].x < -transformed_vertices[0].w &&
		transformed_vertices[1].x < -transformed_vertices[1].w &&
		transformed_vertices[2].x < -transformed_vertices[2].w)
		return;
	if (transformed_vertices[0].y > transformed_vertices[0].w &&
		transformed_vertices[1].y > transformed_vertices[1].w &&
		transformed_vertices[2].y > transformed_vertices[2].w)
		return;
	if (transformed_vertices[0].y < -transformed_vertices[0].w &&
		transformed_vertices[1].y < -transformed_vertices[1].w &&
		transformed_vertices[2].y < -transformed_vertices[2].w)
		return;
	if (transformed_vertices[0].z < -transformed_vertices[0].w &&
		transformed_vertices[1].z < -transformed_vertices[1].w &&
		transformed_vertices[2].z < -transformed_vertices[2].w)
		return;

	gl_Layer = face + int(l) * 6;
	for (int j = 0; j < 3; ++j) {
		vout.uv = vin[j].uv;
		gl_Position = transformed_vertices[j];

		EmitVertex();
	}

	EndPrimitive();
}

void main() {
	int sproj_instance_id = vin[0].instanceIdx;
	uint draw_id = vin[0].drawIdx;
	uint16_t ll_id = sproj_id_to_llid_tt[draw_id].ll_idx[sproj_instance_id];

	light_descriptor ld = light_buffer[ll[ll_id]];

	uint32_t face_mask = ld.shadow_face_mask;
	if (face_mask == 0)
		return;

	vec4 light_pos = vec4(ld.position_direction.xyz, 0);
	float light_range = min(ld.effective_range, far);
	float light_range2 = light_range * light_range;

	vec3 N = (vin[0].normal + vin[1].normal + vin[2].normal) / 3.f;
	vec3 V = light_pos.xyz - gl_in[0].gl_Position.xyz;

	if (dot(N,V) >= 0)
		return;

	vec4 vertices[3];

	int out_of_range = 0;
	for (int j = 0; j < 3; ++j) {
		vec4 P = gl_in[j].gl_Position - light_pos;
		if (dot(P.xyz, P.xyz) >= light_range2)
			++out_of_range;

		vertices[j] = P;
	}
	if (out_of_range == 3)
		return;

	vout.matIdx = vin[0].matIdx;
	for (int face = 0; face < 6; ++face) {
		if ((face_mask & (1 << face)) != 0)
			process(face, ll_id, vertices);
	}
}
