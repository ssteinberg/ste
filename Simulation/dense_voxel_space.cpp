
#include "stdafx.h"
#include "dense_voxel_space.h"

using namespace StE::Graphics;

void dense_voxel_space::create_dense_voxel_space(float voxel_size_factor) {
	float fb_pixel_size = 2.f * glm::tan(.5f * ctx.get_fov()) * ctx.get_near_clip() / ctx.get_backbuffer_size().x;
	float space_size = ctx.get_far_clip();

	float fb_pixel_theta = glm::atan(fb_pixel_size / ctx.get_near_clip());
	float l = fb_pixel_size / voxel_size_factor / glm::tan(fb_pixel_theta);

	steps = glm::floorPowerOfTwo(static_cast<unsigned>(glm::log2(space_size / l)));
	mipmaps = glm::min<std::size_t>(steps, glm::ceil(glm::log2(size.x / tile_size.x) - 1));
	step_size = size.x / steps;
	voxel_size = space_size / size.x;

	space_radiance = std::make_unique<LLR::texture_sparse_3d>(space_format_radiance, size, mipmaps, tile_size, 0);
	space_data = std::make_unique<LLR::texture_sparse_3d>(space_format_data, size, mipmaps, tile_size, 0);

	auto tiles_per_step = step_size / tile_size.x;
	auto center = size / 2u;
	for (std::size_t i = 0; i < mipmaps; ++i, center /= 2u) {
		auto min = decltype(size)(glm::max(glm::ivec3(center - tiles_per_step * tile_size), { 0,0,0 }));
		auto max = glm::min(center + tiles_per_step * tile_size, center * 2u);

		space_radiance->commit_tiles(min, max - min, i);
		space_data->commit_tiles(min, max - min, i);
	}

	for (unsigned i = 0; i < mipmaps; ++i) {
		(*space_radiance)[i].make_resident();
		(*space_data)[i].make_resident();
	}
	space_data->get_texture_handle().make_resident();
	space_radiance->get_texture_handle().make_resident();

	update_shader_voxel_uniforms(*voxelizer_program);
}

void dense_voxel_space::clear_space() const {
	glm::vec4 clear_data(.0f);

	auto tiles_per_step = step_size / tile_size.x;
	auto center = size / 2u;
	for (std::size_t i = 0; i < mipmaps; ++i, center /= 2u) {
		auto min = decltype(size)(glm::max(glm::ivec3(center - tiles_per_step * tile_size), { 0,0,0 }));
		auto max = glm::min(center + tiles_per_step * tile_size, center * 2u);

		space_radiance->clear(&clear_data, min, max - min, i);
		space_data->clear(&clear_data, min, max - min, i);
	}
}
