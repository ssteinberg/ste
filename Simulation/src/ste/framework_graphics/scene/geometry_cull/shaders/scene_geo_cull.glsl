
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 128) in;

#include "girenderer_transform_buffer.glsl"
#include "indirect.glsl"

#include "light.glsl"
#include "light_cascades.glsl"
#include "shadow_projection_instance_to_ll_idx_translation.glsl"

#include "intersection.glsl"

#include "mesh_descriptor.glsl"

layout(std430, binding = 14) restrict readonly buffer mesh_data {
	mesh_descriptor mesh_descriptor_buffer[];
};
layout(std430, binding = 15) restrict readonly buffer mesh_draw_params_data {
	mesh_draw_params mesh_draw_params_buffer[];
};

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};

layout(shared, binding = 4) restrict readonly buffer ll_counter_data {
	uint32_t ll_counter;
};
layout(shared, binding = 5) restrict readonly buffer ll_data {
	uint16_t ll[];
};
layout(shared, binding = 6) restrict readonly buffer directional_lights_cascades_data {
	light_cascade_descriptor directional_lights_cascades[];
};

layout(binding = 0) uniform atomic_uint counter;
layout(std430, binding = 10) restrict writeonly buffer idb_data {
	IndirectMultiDrawElementsCommand idb[];
};
layout(std430, binding = 0) restrict writeonly buffer sidb_data {
	IndirectMultiDrawElementsCommand sidb[];
};
layout(std430, binding = 1) restrict writeonly buffer dsidb_data {
	IndirectMultiDrawElementsCommand dsidb[];
};
layout(std430, binding = 8) restrict writeonly buffer shadow_projection_instance_to_ll_idx_translation_data {
	shadow_projection_instance_to_ll_idx_translation sproj_id_to_llid_tt[];
};
layout(std430, binding = 9) restrict writeonly buffer directional_shadow_projection_instance_to_ll_idx_translation_data {
	directional_shadow_projection_instance_to_ll_idx_translation dsproj_id_to_llid_tt[];
};

void main() {
	int draw_id = int(gl_GlobalInvocationID.x);
	if (draw_id >= mesh_descriptor_buffer.length())
		return;

	mesh_descriptor md = mesh_descriptor_buffer[draw_id];

	vec3 center = transform_view(transform_model(md, md.bounding_sphere.xyz));
	float radius = md.bounding_sphere.w;
	
	uint shadow_instance_count = 0;
	uint dir_shadow_instance_count = 0;

	for (int i = 0; i < ll_counter; ++i) {
		uint16_t light_idx = ll[i];
		light_descriptor ld = light_buffer[light_idx];
		vec3 l = ld.transformed_position;
		
		if (ld.type == LightTypeDirectional) {
			uint32_t cascade_idx = light_get_cascade_descriptor_idx(ld);
			light_cascade_descriptor cascade_descriptor = directional_lights_cascades[cascade_idx];

			for (int cascade=0; cascade<directional_light_cascades; ++cascade) {
				vec2 z_limits;
				float cascade_eye_dist;
				float recp_viewport;
				light_cascade_data(cascade_descriptor, cascade, z_limits, cascade_eye_dist, recp_viewport);
				mat3x4 M = light_cascade_projection(cascade_descriptor, cascade, l, cascade_eye_dist, recp_viewport);
				
				vec3 center_in_cascade_space  = vec4(center, 1) * M;
				if (any(lessThan(abs(center_in_cascade_space.xy), vec2(1.f + radius * recp_viewport)))) {
					dsproj_id_to_llid_tt[draw_id].ll_idx[dir_shadow_instance_count] = uint16_t(i);
					++dir_shadow_instance_count;
					break;
				}
			}
		}
		else {
			float lr = ld.effective_range;

			if (collision_sphere_sphere(l, lr, center, radius)) {
				sproj_id_to_llid_tt[draw_id].ll_idx[shadow_instance_count] = uint16_t(i);
				++shadow_instance_count;
			}
		}
	}

	// Write to scene and shadows indirect draw buffers
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

		c.instance_count = dir_shadow_instance_count;
		dsidb[idx] = c;
	}
}
