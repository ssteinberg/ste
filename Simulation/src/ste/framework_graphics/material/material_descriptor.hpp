// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "texture_handle.hpp"

namespace StE {
namespace Graphics {

struct material_descriptor {
	Core::texture_handle basecolor_handle;
	Core::texture_handle cavity_handle;
	Core::texture_handle normal_handle;
	Core::texture_handle mask_handle;

	glm::vec3 emission{ .0f, .0f, .0f };
	float roughness{ .5f };
	float anisotropy_ratio{ 1.f };
	float metallic{ .0f };
	float F0{ .04f };
	float sheen{ .0f };
};

}
}
