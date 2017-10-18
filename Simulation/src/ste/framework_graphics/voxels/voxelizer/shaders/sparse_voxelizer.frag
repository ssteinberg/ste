
#type frag
#version 450

#include <material.glsl>
#include <voxels.glsl>

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

	// Discard voxel fragments outside the AABB or voxel grid
	vec3 v = P / voxel_world + .5f;
	if (any(greaterThan(gl_FragCoord.xy, fragment.max_aabb.xy)) ||
		any(greaterThanEqual(v, vec3(1))) ||
		any(lessThan(v, vec3(0))))
		return;

	// Partial derivative for material texture look-ups
	vec2 dUVdx = dFdx(uv) * 4;
	vec2 dUVdy = dFdy(uv) * 4;
	
	// Discard voxels that are masked by material
	if (material_is_masked(md, uv, dUVdx, dUVdy))
		return;

	// Add to voxel list
	uint voxel_list_idx = atomicAdd(voxel_list_buffer_size, 1);
	voxel_list_element_t element;
	encode_voxel_list_element(element, v, N, material_id);

	voxel_list_buffer[voxel_list_idx] = element;
}
