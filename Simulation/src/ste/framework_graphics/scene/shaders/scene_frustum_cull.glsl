
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 128) in;

#include "girenderer_matrix_buffer.glsl"
#include "indirect.glsl"
#include "light.glsl"
#include "mesh_descriptor.glsl"

layout(std430, binding = 1) restrict readonly buffer mesh_data {
	mesh_descriptor mesh_descriptor_buffer[];
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

void main() {
	int draw_id = int(gl_GlobalInvocationID.x);
	if (draw_id >= mesh_descriptor_buffer.length())
		return;

	mesh_descriptor md = mesh_descriptor_buffer[draw_id];

	vec3 center = (view_matrix_buffer.view_matrix * (md.model * vec4(md.bounding_sphere.xyz, 1))).xyz;
	float radius = md.bounding_sphere.w;

	bool include = false;
	for (int i = 0; i < ll_counter; ++i) {
		uint16_t light_idx = ll[i];

		vec3 lc = light_transform_buffer[light_idx].xyz;
		float lr = light_buffer[light_idx].effective_range;

		vec3 v = lc - center;
		float d = lr + radius;
		if (dot(v,v) < d*d) {
			include = true;
			break;
		}
	}

	if (include) {
		uint idx = atomicCounterIncrement(counter);

		IndirectMultiDrawElementsCommand c;
		c.count = md.count;
		c.instance_count = 1;
		c.first_index = md.first_index;
		c.base_vertex = md.base_vertex;
		c.base_instance = draw_id;

		idb[idx] = c;
	}
}
