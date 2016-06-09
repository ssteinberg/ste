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

	glm::vec3 emission{ .0f, .0f, .0f };

	std::uint32_t layer_id{ material_layer_none };
};

}
}
