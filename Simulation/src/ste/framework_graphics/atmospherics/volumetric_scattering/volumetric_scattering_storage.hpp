// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <texture.hpp>
#include <surface_factory.hpp>
#include <linked_light_lists.hpp>

#include <signal.hpp>
#include <alias.hpp>
#include <lib/unique_ptr.hpp>

namespace ste {
namespace graphics {

class volumetric_scattering_storage {
private:
	static constexpr int tile_size = linked_light_lists::lll_image_res_multiplier;
	static constexpr int depth_tiles = 256;

private:
	alias<const ste_context> ctx;

	glm::uvec3 extent;
	ste_resource<gl::texture<gl::image_type::image_3d>> volume;

	signal<> storage_modified_signal;

public:
	volumetric_scattering_storage(const ste_context &ctx,
								  const glm::uvec2 &framebuffer_extent)
		: ctx(ctx),
		extent(glm::uvec3{ framebuffer_extent.x / tile_size, framebuffer_extent.y / tile_size, depth_tiles }),
		volume(ctx, resource::surface_factory::image_empty_3d<gl::format::r16g16b16a16_sfloat>(ctx,
																							   gl::image_usage::sampled | gl::image_usage::storage,
																							   gl::image_layout::transfer_dst_optimal,
																							   extent))
		//		volume_sampler(Core::texture_filtering::Linear, Core::texture_filtering::Linear,
		//																	Core::texture_wrap_mode::ClampToEdge, Core::texture_wrap_mode::ClampToEdge, 16),
		//													 depth_sampler(Core::texture_filtering::Nearest, Core::texture_filtering::Nearest, Core::texture_filtering::Nearest,
		//																   Core::texture_wrap_mode::ClampToEdge, Core::texture_wrap_mode::ClampToEdge) {
		//		volume_sampler.set_wrap_r(Core::texture_wrap_mode::ClampToEdge);

	{}
	~volumetric_scattering_storage() noexcept {}

	volumetric_scattering_storage(volumetric_scattering_storage&&) = default;

	void resize(const glm::uvec2 &s) {
		auto tiles = glm::uvec3{ s.x / tile_size,
								 s.y / tile_size,
								 depth_tiles };

		if (tiles.x <= 0 || tiles.y <= 0 || tiles == extent)
			return;

		extent = tiles;
		volume = ste_resource<gl::texture<gl::image_type::image_3d>>(ctx.get(),
																	 resource::surface_factory::image_empty_3d<gl::format::r16g16b16a16_sfloat>(ctx.get(),
																																				gl::image_usage::sampled | gl::image_usage::storage,
																																				gl::image_layout::transfer_dst_optimal,
																																				extent));

		storage_modified_signal.emit();
	}

	auto& get_tiles_extent() const { return extent; }
	auto& get_volume_texture() const { return *volume; }
	//
	//	auto& get_volume_sampler() const { return volume_sampler; }
	//	auto& get_depth_sampler() const { return depth_sampler; }

	auto& get_storage_modified_signal() const { return storage_modified_signal; }
};

}
}
