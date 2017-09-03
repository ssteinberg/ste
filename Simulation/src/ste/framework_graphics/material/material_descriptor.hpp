//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <std430.hpp>

#include <material_layer_descriptor.hpp>

namespace ste {
namespace graphics {

struct material_descriptor : gl::std430<std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, float, std::uint32_t, std::uint32_t, std::uint32_t> {
public:
	static constexpr std::uint32_t material_has_texture_bit = 0x1 << 0;
	static constexpr std::uint32_t material_has_cavity_map_bit = 0x1 << 1;
	static constexpr std::uint32_t material_has_normal_map_bit = 0x1 << 2;
	static constexpr std::uint32_t material_has_mask_map_bit = 0x1 << 3;

	static constexpr std::uint32_t material_has_subsurface_scattering_bit = 0x1 << 31;

public:
	auto& cavity_handle() { return get<0>(); }
	auto& normal_handle() { return get<1>(); }
	auto& mask_handle() { return get<2>(); }
	auto& texture_handle() { return get<3>(); }

	auto& emission() { return get<4>(); }
	auto& packed_emission_color() { return get<5>(); }

	auto& head_layer_id() { return get<6>(); }

	auto& material_flags() { return get<7>(); }

public:
	void set_emission(const glm::vec4 &c) {
		const auto mag = glm::length(c);
		std::uint32_t pack = 0;

		if (mag > 0.000001f) {
			const auto n = c / mag;
			pack = glm::packUnorm4x8(n);
		}

		packed_emission_color() = pack;
		emission() = mag;
	}
};

}
}
