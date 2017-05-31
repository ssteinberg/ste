// StE
// © Shlomi Steinberg, 2015-2016

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

	lib::unique_ptr<gl::texture<gl::image_type::image_3d>> volume{ nullptr };
	glm::ivec3 size;

	signal<> storage_modified_signal;

public:
	volumetric_scattering_storage(const ste_context &ctx) : ctx(ctx) {
//		volume_sampler(Core::texture_filtering::Linear, Core::texture_filtering::Linear,
//																	Core::texture_wrap_mode::ClampToEdge, Core::texture_wrap_mode::ClampToEdge, 16),
//													 depth_sampler(Core::texture_filtering::Nearest, Core::texture_filtering::Nearest, Core::texture_filtering::Nearest,
//																   Core::texture_wrap_mode::ClampToEdge, Core::texture_wrap_mode::ClampToEdge) {
//		volume_sampler.set_wrap_r(Core::texture_wrap_mode::ClampToEdge);

		resize(size);
	}

	void resize(glm::ivec2 s) {
		if (s.x <= 0 || s.y <= 0)
			return;

		size = glm::ivec3{ s.x / tile_size,
						   s.y / tile_size,
						   depth_tiles };
		auto v = resource::surface_factory::image_empty_3d<gl::format::r16g16b16a16_sfloat>(ctx.get(),
																							gl::image_usage::sampled | gl::image_usage::storage,
																							gl::image_layout::general,
																							size);
		volume = lib::allocate_unique<gl::texture<gl::image_type::image_3d>>(std::move(v).get());

		storage_modified_signal.emit();
	}

	/*void set_depth_maps(Core::texture_2d *dm, Core::texture_2d *downsampled_dm) {
		depth_map = dm;
		downsampled_depth_map = downsampled_dm;
		storage_modified_signal.emit();
	}*/

	auto& get_size() const { return size; }

	auto* get_volume_texture() const { return volume.get(); }
//	auto* get_depth_map() const { return depth_map; }
//	auto* get_downsampled_depth_map() const { return downsampled_depth_map; }
//
//	auto& get_volume_sampler() const { return volume_sampler; }
//	auto& get_depth_sampler() const { return depth_sampler; }

	auto& get_storage_modified_signal() const { return storage_modified_signal; }
};

}
}
