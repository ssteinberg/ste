// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "surface_traits.h"

#include <gli/gli.hpp>

namespace StE {
namespace Graphics {

template <gli::format Fin>
class normal_map_from_height_map {
private:
	using T = LLR::surface_element_type<Fin>::type;

public:
	gli::texture2D operator()(const gli::texture2D &height_map, float height_scale) {
		assert(Fin == height_map.format());

		unsigned components_in = gli::component_count(Fin);

		assert(components_in == 1);

		auto dim = height_map.dimensions();
		gli::texture2D nm(1, gli::format::FORMAT_RGB32_SFLOAT, dim);

		glm::vec3 *data = reinterpret_cast<glm::vec3*>(nm.data());
		const T *heights = reinterpret_cast<const T*>(height_map.data());
		for (unsigned y = 0; y < dim.y; ++y) {
			for (unsigned x = 0; x < dim.x; ++x, ++data) {
				glm::vec3 n;

				T c = heights[dim.x * y + x];
				T u = y + 1 < dim.y ? heights[dim.x * (y + 1) + x] : c;
				T d = y > 0 ? heights[dim.x * (y - 1) + x] : c;
				T r = x + 1 < dim.x ? heights[dim.x * y + x + 1] : c;
				T l = x > 0 ? heights[dim.x * y + x - 1] : c;

				n.z = 1.0f;
				n.y = (static_cast<float>(c - u) * .5f + static_cast<float>(d - c) * .5f) * height_scale;
				n.x = (static_cast<float>(c - r) * .5f + static_cast<float>(l - c) * .5f) * height_scale;

				*data = glm::normalize(n);
			}
		}

		return std::move(nm);
	}
};

}
}
