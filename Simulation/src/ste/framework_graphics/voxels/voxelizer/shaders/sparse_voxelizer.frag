
#type frag
#version 450
#extension GL_ARB_shader_ballot : require

#include <material.glsl>
#include <voxels_voxelize.glsl>

layout(location = 0) in geo_out {
	vec3 P, N;
	vec2 st;
	float transformed_triangle_z;
	flat int material_id;
	flat vec2 max_aabb;
	flat vec3 ortho_projection_mask;
} fragment;

void main() {
	// Read voxel proeprties
	int material_id = fragment.material_id;
	vec2 uv = fragment.st;
	vec3 P = fragment.P;
	vec3 N = fragment.N;
	material_descriptor md = mat_descriptor[material_id];

	// Partial derivative for material texture look-ups
	//vec2 dUVdx = dFdx(uv) * 4;
	//vec2 dUVdy = dFdy(uv) * 4;
	
	// Discard voxels outside the AABB or masked by material
	if (any(greaterThan(gl_FragCoord.xy, fragment.max_aabb.xy)) ||
		material_is_masked(md, uv, vec2(1), vec2(1))) {
		discard;
		return;
	}

	// Voxelize
	voxelize(P);

	// Compute positions at fragment edges
	/*vec3 dPdx = dFdx(P) * .5f;
	vec3 dPdy = dFdy(P) * .5f;

	vec3 P00 = round(P - dPdx - dPdy) * fragment.ortho_projection_mask;
	vec3 P10 = round(P + dPdx - dPdy) * fragment.ortho_projection_mask;
	vec3 P01 = round(P - dPdx + dPdy) * fragment.ortho_projection_mask;
	vec3 P11 = round(P + dPdx + dPdy) * fragment.ortho_projection_mask;

	// Voxelize additional voxels to ensure true 6-plane separable voxel grid
    vec3 voxel = round(P / voxel_grid_resolution);
	vec3 voxel_d1 = voxel * fragment.ortho_projection_mask + fragment.ortho_projection_mask;
	vec3 voxel_d2 = voxel * fragment.ortho_projection_mask - fragment.ortho_projection_mask;

	if (any(bvec4(P00 == voxel_d1, P10 == voxel_d1, P01 == voxel_d1, P11 == voxel_d1))) {
		vec3 P_d1 = P + voxel_grid_resolution * fragment.ortho_projection_mask;
		voxelize(...);
	}

	if (any(bvec4(P00 == voxel_d2, P10 == voxel_d2, P01 == voxel_d2, P11 == voxel_d2))) {
		vec3 P_d2 = P - voxel_grid_resolution * fragment.ortho_projection_mask;
		voxelize(...);
	}*/
}
