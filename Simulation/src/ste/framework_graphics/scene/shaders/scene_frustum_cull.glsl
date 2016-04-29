
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 128) in;

#include "indirect.glsl"
#include "mesh_descriptor.glsl"

layout(binding = 0) uniform atomic_uint counter;
layout(std430, binding = 0) restrict writeonly buffer idb_data {
	IndirectMultiDrawElementsCommand idb[];
};
layout(std430, binding = 1) restrict readonly buffer mesh_data {
	mesh_descriptor mesh_descriptor_buffer[];
};
layout(std430, binding = 12) restrict writeonly buffer id_to_drawid_data {
	uint id_to_drawid[];
};

uniform mat4 view_matrix;
uniform vec4 np, fp, rp, lp, tp, bp;

bool is_sphere_in_frustum(vec3 c, float r) {
	return  dot(np.xyz, c) + np.w > -r &&
			dot(fp.xyz, c) + fp.w > -r &&
			dot(rp.xyz, c) + rp.w > -r &&
			dot(lp.xyz, c) + lp.w > -r &&
			dot(tp.xyz, c) + tp.w > -r &&
			dot(bp.xyz, c) + bp.w > -r;
}

void main() {
	int draw_id = int(gl_GlobalInvocationID.x);
	if (draw_id >= mesh_descriptor_buffer.length())
		return;

	mesh_descriptor md = mesh_descriptor_buffer[draw_id];

	vec4 center = view_matrix * md.model * vec4(md.bounding_sphere.xyz, 1);
	float radius = md.bounding_sphere.w;

	if (is_sphere_in_frustum(center.xyz, radius)) {
		uint idx = atomicCounterIncrement(counter);

		IndirectMultiDrawElementsCommand c;
		c.count = md.count;
		c.instance_count = 1;
		c.first_index = md.first_index;
		c.base_vertex = md.base_vertex;
		c.base_instance = 0;

		id_to_drawid[idx] = draw_id;
		idb[idx] = c;
	}
}
