// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "signal.hpp"

#include "Texture2D.hpp"
#include "Texture3D.hpp"
#include "Sampler.hpp"

#include "linked_light_lists.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class volumetric_scattering_storage {
private:
	constexpr static int tile_size = linked_light_lists::lll_image_res_multiplier;
	constexpr static int depth_tiles = 256;

private:
	Core::Texture2D *depth_map;
	std::unique_ptr<Core::Texture3D> volume;

	Core::Sampler volume_sampler;
	Core::SamplerMipmapped depth_sampler;

	float scatter_phase_aniso_coefficient;

	glm::ivec3 size;

	signal<> storage_modified_signal;

public:
	volumetric_scattering_storage(glm::ivec2 size,
								  float scatter_phase_aniso_coefficient = .15f) : volume_sampler(Core::TextureFiltering::Linear, Core::TextureFiltering::Linear,
																								 Core::TextureWrapMode::ClampToEdge, Core::TextureWrapMode::ClampToEdge, 16),
																				  depth_sampler(Core::TextureFiltering::Nearest, Core::TextureFiltering::Nearest, Core::TextureFiltering::Nearest,
																								Core::TextureWrapMode::ClampToEdge, Core::TextureWrapMode::ClampToEdge) {
		volume_sampler.set_wrap_r(Core::TextureWrapMode::ClampToEdge);

		resize(size);
	}

	void resize(glm::ivec2 s) {
		if (s.x <= 0 || s.y <= 0)
			return;

		size = glm::ivec3{ s.x / tile_size,
						   s.y / tile_size,
						   depth_tiles };

		volume = std::make_unique<Core::Texture3D>(gli::format::FORMAT_RGBA16_SFLOAT_PACK16, size);

		storage_modified_signal.emit();
	}

	void set_depth_map(Core::Texture2D *dm) {
		depth_map = dm;
		storage_modified_signal.emit();
	}

	auto& get_size() const { return size; }

	auto get_scattering_phase_anisotropy_coefficient() const { return scatter_phase_aniso_coefficient; }

	auto* get_volume_texture() const { return volume.get(); }
	auto* get_depth_map() const { return depth_map; }

	auto& get_volume_sampler() const { return volume_sampler; }
	auto& get_depth_sampler() const { return depth_sampler; }

	auto& get_storage_modified_signal() const { return storage_modified_signal; }
};

}
}
