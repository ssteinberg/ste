
#include "stdafx.h"
#include "dense_voxel_space.h"

using namespace StE::Graphics;

void dense_voxel_space::create_dense_voxel_space() {
	float fb_pixel_size = 2.f * glm::tan(.5f * ctx.get_fov()) * ctx.get_near_clip() / ctx.get_backbuffer_size().x;
	float space_size = ctx.get_far_clip();

	float fb_pixel_theta = glm::atan(fb_pixel_size / ctx.get_near_clip());
	float l = fb_pixel_size / voxel_size_factor / glm::tan(fb_pixel_theta);

	steps = glm::floorPowerOfTwo(static_cast<unsigned>(glm::log2(space_size / l)));
	mipmaps = glm::min<std::size_t>(steps, glm::ceil(glm::log2(size.x / tile_size.x) + 1));
	step_size = size.x / steps;
	voxel_size = space_size / size.x;

	assert(mipmaps <= 8 && "GLSL 4.5 bindless uniform array size limit");

	space = std::make_unique<LLR::texture_sparse_3d>(space_format, size, mipmaps, tile_size, 0);

	auto tiles_per_step = step_size / tile_size.x;
	auto center = size / 2u;
	for (std::size_t i = 0; i < mipmaps; ++i, tiles_per_step /= 2, center /= 2u) {
		tiles_per_step = glm::max(tiles_per_step, 1u);

		auto min = decltype(size)(glm::max(glm::ivec3(center - tiles_per_step * tile_size), { 0,0,0 }));
		auto max = glm::min(center + tiles_per_step * tile_size, center * 2u);

		space->commit_tiles(min, max - min, i);
	}

	update_shader_voxel_uniforms(*voxelizer_program);
}
