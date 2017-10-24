
#type frag
#version 450

#include <material.glsl>
#include <material_layer_unpack.glsl>
#include <voxels.glsl>


layout(std430, set=1, binding=2) restrict buffer voxel_list_binding {
	voxel_list_element_t voxel_list_buffer[];
};
layout(std430, set=1, binding=3) restrict buffer voxel_list_counter_binding {
	uint voxel_list_buffer_size;
};


layout(location = 0) in geo_out {
	vec3 P, N;
	vec2 st;
	flat int material_id;
	flat vec2 max_aabb;
} fragment;

void main() {
	// Read voxel fragment proeprties
	int material_id = fragment.material_id;
	vec2 uv = fragment.st;
	vec3 P = fragment.P;
	vec3 N = fragment.N;
	material_descriptor md = mat_descriptor[material_id];
	
	// Discard voxels that are masked by material
	// Discard voxel fragments outside the AABB or voxel grid
	vec3 v = P / voxel_world + .5f;
	if (material_is_masked(md, uv) ||
		any(greaterThan(gl_FragCoord.xy, fragment.max_aabb.xy)) ||
		any(greaterThanEqual(v, vec3(1))) ||
		any(lessThan(v, vec3(0))))
		return;
	
	// Read material data (ignoring multi-layered materials)
	material_layer_descriptor head_layer = mat_layer_descriptor[md.head_layer];
	material_layer_unpacked_descriptor descriptor = material_layer_unpack(head_layer, uv);
	vec4 rgba = material_base_texture(md, uv);
	float roughness = descriptor.roughness;

	voxel_list_element_t element;
	// Encode voxel data and element position
	element.data = encode_voxel_data(N, roughness, rgba);
	element.node_x = v.x;
	element.node_y = v.y;
	element.node_z = v.z;
	
	// Add to voxel list
	uint voxel_list_idx = atomicAdd(voxel_list_buffer_size, 1);
	voxel_list_buffer[voxel_list_idx] = element;
}
