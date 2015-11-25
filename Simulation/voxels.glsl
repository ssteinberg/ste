
uniform uint voxels_step_texels;
uniform uint voxels_steps;
uniform float voxels_voxel_texel_size;
uniform uint voxels_texture_levels;

layout(bindless_image, rgba32f) uniform image3D voxel_levels[8];

uint voxel_level(vec3 P) {
	P = abs(P);
	float l = max(P.x, max(P.y, P.z));
	return min(uint(floor(l / voxels_voxel_texel_size / voxels_step_texels)), voxels_texture_levels);
}

float voxel_size(vec3 P) {
	return pow(2, voxel_level(P)) * voxels_voxel_texel_size;
}

void voxelize(vec3 P, vec4 color) {
	uint level = voxel_level(P);
	float size = voxel_size(P);

	ivec3 vtc = ivec3(round(P / size)) + (imageSize(voxel_levels[level]).x >> 1).xxx;
	imageStore(voxel_levels[level], vtc, color);
}
