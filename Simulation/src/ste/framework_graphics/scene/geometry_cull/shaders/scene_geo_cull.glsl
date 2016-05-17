
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 128) in;

#include "girenderer_transform_buffer.glsl"
#include "indirect.glsl"
#include "light.glsl"
#include "mesh_descriptor.glsl"
#include "shadow_projection_instance_to_ll_idx_translation.glsl"

layout(std430, binding = 14) restrict readonly buffer mesh_data {
	mesh_descriptor mesh_descriptor_buffer[];
};
layout(std430, binding = 15) restrict readonly buffer mesh_draw_params_data {
	mesh_draw_params mesh_draw_params_buffer[];
};
layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};
layout(shared, binding = 3) restrict readonly buffer light_transform_data {
	vec4 light_transform_buffer[];
};
layout(shared, binding = 4) restrict readonly buffer ll_counter_data {
	uint32_t ll_counter;
};
layout(shared, binding = 5) restrict readonly buffer ll_data {
	uint16_t ll[];
};

layout(binding = 0) uniform atomic_uint counter;
layout(std430, binding = 0) restrict writeonly buffer idb_data {
	IndirectMultiDrawElementsCommand idb[];
};
layout(std430, binding = 1) restrict writeonly buffer sidb_data {
	IndirectMultiDrawElementsCommand sidb[];
};
layout(std430, binding = 8) restrict writeonly buffer shadow_projection_instance_to_ll_idx_translation_data {
	shadow_projection_instance_to_ll_idx_translation sproj_id_to_llid_tt[];
};

void main() {
	int draw_id = int(gl_GlobalInvocationID.x);
	if (draw_id >= mesh_descriptor_buffer.length())
		return;

	mesh_descriptor md = mesh_descriptor_buffer[draw_id];

	vec3 center = dquat_mul_vec(view_transform_buffer.view_transform, dquat_mul_vec(md.model_transform, md.bounding_sphere.xyz));
	float radius = md.bounding_sphere.w;

	uint shadow_instance_count = 0;
	for (int i = 0; i < ll_counter; ++i) {
		uint16_t light_idx = ll[i];

		vec3 lc = light_transform_buffer[light_idx].xyz;
		float lr = light_buffer[light_idx].effective_range;

		vec3 v = lc - center;
		float d = lr + radius;
		if (dot(v,v) < d*d) {
			sproj_id_to_llid_tt[draw_id].ll_idx[shadow_instance_count] = uint16_t(i);
			++shadow_instance_count;
		}
	}

	if (shadow_instance_count > 0) {
		uint idx = atomicCounterIncrement(counter);

		IndirectMultiDrawElementsCommand c;
		c.count = mesh_draw_params_buffer[draw_id].count;
		c.instance_count = 1;
		c.first_index = mesh_draw_params_buffer[draw_id].first_index;
		c.base_vertex = mesh_draw_params_buffer[draw_id].base_vertex;
		c.base_instance = draw_id;

		idb[idx] = c;

		c.instance_count = shadow_instance_count;
		sidb[idx] = c;
	}
}
