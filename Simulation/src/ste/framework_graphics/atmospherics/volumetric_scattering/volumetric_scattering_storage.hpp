// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "signal.hpp"

#include "texture_2d.hpp"
#include "texture_3d.hpp"
#include "sampler.hpp"

#include "linked_light_lists.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class volumetric_scattering_storage {
private:
	static constexpr int tile_size = linked_light_lists::lll_image_res_multiplier;
	static constexpr int depth_tiles = 256;

private:
	Core::texture_2d *depth_map{ nullptr };
	Core::texture_2d *downsampled_depth_map{ nullptr };
	std::unique_ptr<Core::texture_3d> volume{ nullptr };

	Core::sampler volume_sampler;
	Core::sampler_mipmapped depth_sampler;

	glm::ivec3 size;

	signal<> storage_modified_signal;

public:
	volumetric_scattering_storage(glm::ivec2 size) : volume_sampler(Core::texture_filtering::Linear, Core::texture_filtering::Linear,
																	Core::texture_wrap_mode::ClampToEdge, Core::texture_wrap_mode::ClampToEdge, 16),
													 depth_sampler(Core::texture_filtering::Nearest, Core::texture_filtering::Nearest, Core::texture_filtering::Nearest,
																   Core::texture_wrap_mode::ClampToEdge, Core::texture_wrap_mode::ClampToEdge) {
		volume_sampler.set_wrap_r(Core::texture_wrap_mode::ClampToEdge);

		resize(size);
	}

	void resize(glm::ivec2 s) {
		if (s.x <= 0 || s.y <= 0)
			return;

		size = glm::ivec3{ s.x / tile_size,
						   s.y / tile_size,
						   depth_tiles };

		volume = std::make_unique<Core::texture_3d>(gli::format::FORMAT_RGBA32_SFLOAT_PACK32, size);

		storage_modified_signal.emit();
	}

	void set_depth_maps(Core::texture_2d *dm, Core::texture_2d *downsampled_dm) {
		depth_map = dm;
		downsampled_depth_map = downsampled_dm;
		storage_modified_signal.emit();
	}

	auto& get_size() const { return size; }

	auto* get_volume_texture() const { return volume.get(); }
	auto* get_depth_map() const { return depth_map; }
	auto* get_downsampled_depth_map() const { return downsampled_depth_map; }

	auto& get_volume_sampler() const { return volume_sampler; }
	auto& get_depth_sampler() const { return depth_sampler; }

	auto& get_storage_modified_signal() const { return storage_modified_signal; }
};

}
}
