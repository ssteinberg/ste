// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <format_type_traits.hpp>

#include <limits>

namespace ste {
namespace graphics {

template <gl::format Format, bool height_in_alpha = true>
class normal_map_from_height_map {
private:
	using T = typename gl::format_traits<Format>::element_type;

public:
	gli::texture2d operator()(const gli::texture2d &height_map, float height_scale) {
		// Sanity
		assert(gl::format_traits<Format>::gli_format == height_map.format());
		assert(gl::format_traits<Format>::components == 1);

		const glm::ivec2 dim{ height_map.extent().x, height_map.extent().y };
		gli::texture2d nm(height_in_alpha ? gli::format::FORMAT_RGBA32_SFLOAT_PACK32 : gli::format::FORMAT_RGB32_SFLOAT_PACK32, dim, 1);

		// Generate normal map
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

				float c = static_cast<float>(ic);
				float u = static_cast<float>(iu);
				float d = static_cast<float>(id);
				float l = static_cast<float>(il);
				float r = static_cast<float>(ir);

				if (gl::format_traits<Format>::is_normalized_integer) {
					c /= static_cast<float>(std::numeric_limits<T>::max());
					u /= static_cast<float>(std::numeric_limits<T>::max());
					d /= static_cast<float>(std::numeric_limits<T>::max());
					l /= static_cast<float>(std::numeric_limits<T>::max());
					r /= static_cast<float>(std::numeric_limits<T>::max());
				}

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
