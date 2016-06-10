// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "texture_handle.hpp"

namespace StE {
namespace Graphics {

static constexpr std::uint32_t material_layer_none = 0xFFFFFFFF;

struct material_layer_descriptor {
	Core::texture_handle basecolor_handle;
	
	float thickness{ .0f };

	float roughness{ .5f };
	float anisotropy_ratio{ 1.f };
	float metallic{ .0f };
	float ior{ 1.5f };

	float sheen_ratio{ .0f };
	float sheen_power{ 5.f };

	std::uint32_t next_layer_id{ material_layer_none };

	float _unused[2];
};

}
}
