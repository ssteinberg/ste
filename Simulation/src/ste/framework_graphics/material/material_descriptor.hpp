// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "texture_handle.hpp"

#include "material_layer_descriptor.hpp"

namespace StE {
namespace Graphics {

struct material_descriptor {
	Core::texture_handle cavity_handle;
	Core::texture_handle normal_handle;
	Core::texture_handle mask_handle;
	Core::texture_handle texture_handle;

	float emission{ .0f };
	std::uint32_t packed_emission_color{ 0 };

	std::uint32_t head_layer_id{ material_layer_none };

	float _unused;

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
