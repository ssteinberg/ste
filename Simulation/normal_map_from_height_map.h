// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "surface_traits.h"
#include "surface_element_cast.h"

#include <gli/gli.hpp>

namespace StE {
namespace Graphics {

template <gli::format Fin>
class normal_map_from_height_map {
private:
	using T = LLR::surface_element_type<Fin>::type;

public:
	gli::texture2D operator()(const gli::texture2D &height_map, float height_scale, bool height_in_alpha = true) {
		assert(Fin == height_map.format());

		unsigned components_in = gli::component_count(Fin);

		assert(components_in == 1);

		auto dim = height_map.dimensions();
		gli::texture2D nm(1, height_in_alpha ? gli::format::FORMAT_RGBA32_SFLOAT : gli::format::FORMAT_RGB32_SFLOAT, dim);

		float *data = reinterpret_cast<float*>(nm.data());
		const T *heights = reinterpret_cast<const T*>(height_map.data());
		for (unsigned y = 0; y < dim.y; ++y) {
			for (unsigned x = 0; x < dim.x; ++x) {
				glm::vec3 n;

				T ic = heights[dim.x * y + x];
				T iu = y + 1 < dim.y ? heights[dim.x * (y + 1) + x] : ic;
				T id = y > 0 ? heights[dim.x * (y - 1) + x] : ic;
				T ir = x + 1 < dim.x ? heights[dim.x * y + x + 1] : ic;
				T il = x > 0 ? heights[dim.x * y + x - 1] : ic;

				float c = LLR::surface_element_cast<Fin, gli::format::FORMAT_R32_SFLOAT>(ic);
				float u = LLR::surface_element_cast<Fin, gli::format::FORMAT_R32_SFLOAT>(iu);
				float d = LLR::surface_element_cast<Fin, gli::format::FORMAT_R32_SFLOAT>(id);
				float l = LLR::surface_element_cast<Fin, gli::format::FORMAT_R32_SFLOAT>(il);
				float r = LLR::surface_element_cast<Fin, gli::format::FORMAT_R32_SFLOAT>(ir);

				n.z = 1.0f;
				n.y = ((c - d) * .5f + (u - c) * .5f) * height_scale;
				n.x = ((c - r) * .5f + (l - c) * .5f) * height_scale;

				if (!height_in_alpha) {
					*reinterpret_cast<glm::vec3*>(data) = glm::normalize(n);
					data += 3;
				}
				else {
					*reinterpret_cast<glm::vec4*>(data) = glm::vec4(glm::normalize(n), c);
					data += 4;
				}
			}
		}

		return std::move(nm);
	}
};

}
}
