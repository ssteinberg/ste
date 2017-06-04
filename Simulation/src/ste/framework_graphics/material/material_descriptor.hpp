//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <material_layer_descriptor.hpp>

namespace ste {
namespace graphics {

struct material_descriptor {
public:
	static constexpr std::uint32_t material_has_texture_bit = 0x1 << 0;
	static constexpr std::uint32_t material_has_cavity_map_bit = 0x1 << 1;
	static constexpr std::uint32_t material_has_normal_map_bit = 0x1 << 2;
	static constexpr std::uint32_t material_has_mask_map_bit = 0x1 << 3;

	static constexpr std::uint32_t material_has_subsurface_scattering_bit = 0x1 << 31;

public:
	std::uint32_t cavity_handle;
	std::uint32_t normal_handle;
	std::uint32_t mask_handle;
	std::uint32_t texture_handle;

	float emission{ .0f };
	std::uint32_t packed_emission_color{ 0 };

	std::uint32_t head_layer_id{ material_layer_none };

	std::uint32_t material_flags{ 0 };

public:
	void set_emission(const glm::vec4 &c) {
		auto mag = glm::length(c);
		std::uint32_t pack = 0;

		if (mag > 0.000001f) {
			auto n = c / mag;
			pack = glm::packUnorm4x8(n);
		}

		packed_emission_color = pack;
		emission = mag;
	}
};

}
}
