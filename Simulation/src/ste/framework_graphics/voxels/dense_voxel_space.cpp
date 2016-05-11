
#include "stdafx.hpp"
#include "dense_voxel_space.hpp"

using namespace StE::Graphics;

constexpr gli::format dense_voxel_space::space_format_radiance;
constexpr gli::format dense_voxel_space::space_format_data;
constexpr int dense_voxel_space::voxel_steps_multiplier;

dense_voxel_space::dense_voxel_space(const StEngineControl &ctx, std::size_t max_size, float voxel_size_factor) : ctx(ctx) {
	auto ts = Core::texture_sparse_3d::page_sizes(space_format_radiance)[0];

	size = static_cast<decltype(size)>(glm::min<int>(Core::texture_sparse_3d::max_size(), max_size));
	tile_size = static_cast<decltype(tile_size)>(glm::max(ts.x, glm::max(ts.y, ts.z)));

	voxelizer_program = ctx.glslprograms_pool().fetch_program_task({ "voxelizer.vert", "voxelizer.frag", "voxelizer.geom" })();
	voxelizer_upsampler_program = ctx.glslprograms_pool().fetch_program_task({ "voxelizer_upsampler.glsl" })();

	voxelizer_output = std::make_unique<Core::RenderTarget>(gli::format::FORMAT_R8_UNORM_PACK8, glm::ivec2{ size.x, size.y });
	voxelizer_fbo[0] = *voxelizer_output;

	sampler.set_min_filter(Core::TextureFiltering::Linear);
	sampler.set_mag_filter(Core::TextureFiltering::Linear);
	sampler.set_mipmap_filter(Core::TextureFiltering::Nearest);
	sampler.set_wrap_s(Core::TextureWrapMode::ClampToBorder);
	sampler.set_wrap_t(Core::TextureWrapMode::ClampToBorder);
	sampler.set_wrap_r(Core::TextureWrapMode::ClampToBorder);

	create_dense_voxel_space(voxel_size_factor);
	projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([=](const glm::mat4 &, float, float) {
		create_dense_voxel_space(voxel_size_factor);
	});
	ctx.signal_projection_change().connect(projection_change_connection);
}

void dense_voxel_space::create_dense_voxel_space(float voxel_size_factor) {
	handles_radiance.clear();
	handles_data.clear();

	float fb_pixel_size = 2.f * glm::tan(.5f * ctx.get_fov()) * ctx.get_near_clip() / ctx.get_backbuffer_size().x;
	space_size = 10000.f; // far clip

	float fb_pixel_theta = glm::atan(fb_pixel_size / ctx.get_near_clip());
	float l = fb_pixel_size / voxel_size_factor / glm::tan(fb_pixel_theta);

	auto min_steps = static_cast<int>(glm::floorPowerOfTwo(glm::log2(size.x / 2 / tile_size.x)));
	auto desired_steps = static_cast<int>(glm::floorPowerOfTwo(static_cast<unsigned>(glm::log2(space_size / l))));
	steps = glm::min<int>(min_steps, desired_steps);
	mipmaps = glm::min<int>(steps + 3, static_cast<int>(glm::log2(size.x)));
	voxel_size = space_size / size.x * 2;
	tiles_per_step = static_cast<int>(size.x / (2 * tile_size.x * glm::pow(2.f, steps - 1)));
	step_size = tiles_per_step * tile_size;

	space_radiance = std::make_unique<Core::texture_sparse_3d>(space_format_radiance, size, mipmaps, tile_size, 0);
	space_data = std::make_unique<Core::texture_sparse_3d>(space_format_data, size, mipmaps, tile_size, 0);

	auto center = size / 2;
	for (int i = 0; i < mipmaps; ++i, center /= 2) {
		auto min = glm::max(glm::ivec3(center - step_size), { 0,0,0 });
		auto max = glm::min(center + step_size, center * 2);

		space_radiance->commit_tiles(min, max - min, i);
		space_data->commit_tiles(min, max - min, i);

//		if (i>0) {
//			auto delta = (center - min) / 2u;
//			delta = decltype(size)(glm::ceil(glm::vec3(delta) / static_cast<float>(tile_size.x) + glm::vec3(1))) * tile_size.x;
//			min = min + delta;
//			max = max - delta;
//
//			if (max.x > min.x) {
//				space_radiance->uncommit_tiles(min, max - min, i);
//				space_data->uncommit_tiles(min, max - min, i);
//			}
//		}

		auto ih = (*space_radiance)[i].get_image_handle();
		auto dh = (*space_data)[i].get_image_handle();

		ih.make_resident();
		dh.make_resident();

		handles_radiance.push_back(ih);
		handles_data.push_back(dh);
	}
	radiance_texture_handle = space_radiance->get_texture_handle(sampler);
	data_texture_handle = space_data->get_texture_handle(sampler);
	radiance_texture_handle.make_resident();
	data_texture_handle.make_resident();

	update_shader_voxel_uniforms(*voxelizer_program);
	update_shader_voxel_uniforms(*voxelizer_upsampler_program);
}

void dense_voxel_space::update_shader_voxel_uniforms(const Core::GLSLProgram &prg) const {
	prg.set_uniform("voxels_step_texels", step_size.x);
	prg.set_uniform("voxels_voxel_texel_size", voxel_size);
	prg.set_uniform("voxels_texture_levels", mipmaps);
	prg.set_uniform("voxels_texture_size", size.x);
	prg.set_uniform("voxels_world_size", space_size);
	prg.set_uniform("voxel_radiance_levels_handles", handles_radiance);
	prg.set_uniform("voxel_data_levels_handles", handles_data);
	prg.set_uniform("voxel_space_radiance", radiance_texture_handle);
	prg.set_uniform("voxel_space_data", data_texture_handle);
}

void dense_voxel_space::clear_space() const {
	glm::vec4 clear_data(.0f);

	auto center = size / 2;
	for (int i = 0; i < mipmaps; ++i, center /= 2) {
		auto min = glm::max(glm::ivec3(center - step_size), { 0,0,0 });
		auto max = glm::min(center + step_size, center * 2);

		space_radiance->clear(&clear_data, min, max - min, i);
		space_data->clear(&clear_data, min, max - min, i);
	}
}
