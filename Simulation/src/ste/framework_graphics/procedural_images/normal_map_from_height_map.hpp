// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "surface_traits.hpp"
#include "surface_element_cast.hpp"

namespace StE {
namespace Graphics {

template <gli::format Fin, bool height_in_alpha = true>
class normal_map_from_height_map {
private:
	using T = typename Core::surface_element_type<Fin>::type;

public:
	gli::texture2d operator()(const gli::texture2d &height_map, float height_scale) {
		assert(Fin == height_map.format());

		unsigned components_in = gli::component_count(Fin);

		assert(components_in == 1);

		glm::ivec2 dim{ height_map.extent().x, height_map.extent().y };
		gli::texture2d nm(height_in_alpha ? gli::format::FORMAT_RGBA32_SFLOAT_PACK32 : gli::format::FORMAT_RGB32_SFLOAT_PACK32, dim, 1);

		float *data = reinterpret_cast<float*>(nm.data());
		const T *heights = reinterpret_cast<const T*>(height_map.data());
		for (int y = 0; y < dim.y; ++y) {
			for (int x = 0; x < dim.x; ++x) {
				glm::vec3 n;

				T ic = heights[dim.x * y + x];
				T iu = y + 1 < dim.y ? heights[dim.x * (y + 1) + x] : ic;
				T id = y > 0 ? heights[dim.x * (y - 1) + x] : ic;
				T ir = x + 1 < dim.x ? heights[dim.x * y + x + 1] : ic;
				T il = x > 0 ? heights[dim.x * y + x - 1] : ic;

				float c = Core::surface_element_cast<Fin, gli::format::FORMAT_R32_SFLOAT_PACK32>(ic);
				float u = Core::surface_element_cast<Fin, gli::format::FORMAT_R32_SFLOAT_PACK32>(iu);
				float d = Core::surface_element_cast<Fin, gli::format::FORMAT_R32_SFLOAT_PACK32>(id);
				float l = Core::surface_element_cast<Fin, gli::format::FORMAT_R32_SFLOAT_PACK32>(il);
				float r = Core::surface_element_cast<Fin, gli::format::FORMAT_R32_SFLOAT_PACK32>(ir);

				n.z = 1.0f;
				n.y = ((c - d) * .5f + (u - c) * .5f) * height_scale;
				n.x = ((c - r) * .5f + (l - c) * .5f) * height_scale;

				if (!height_in_alpha) {
					reinterpret_cast<glm::vec3*>(data)[x + y * dim.x] = glm::normalize(n);
				}
				else {
					reinterpret_cast<glm::vec4*>(data)[x + y * dim.x] = glm::vec4(glm::normalize(n), c);
				}
			}
		}

		return std::move(nm);
	}
};

}
}
