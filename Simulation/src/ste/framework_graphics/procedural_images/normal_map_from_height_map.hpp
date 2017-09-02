//  StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <format_type_traits.hpp>
#include <ste_type_traits.hpp>

#include <limits>

namespace ste {
namespace graphics {

template <gl::format Format>
class normal_map_from_height_map {
private:
	static_assert(gl::format_traits<Format>::elements == 3 || gl::format_traits<Format>::elements == 4);
	static constexpr bool has_height_in_alpha_channel = gl::format_traits<Format>::elements == 4;

	using T = typename gl::format_traits<Format>::block_type::common_type;
//	static_assert(is_floating_point_v<T>, "Target format is not a floating-point format");

public:
	template <gl::format src_format>
	auto operator()(const resource::surface_2d<src_format> &height_map, float height_scale) {
		static_assert(gl::format_traits<src_format>::elements == 1, "height_map must be a 1-channel surface");
		static_assert(!gl::format_traits<src_format>::is_compressed && !gl::format_traits<Format>::is_compressed, "Formats must be uncompressed formats");

		auto dim = height_map.extent();
		resource::surface_2d<Format> nm(dim, 1);

		// Generate normal map
		auto nm_level = nm[0];
		auto height_map_level = height_map[0];
		for (std::uint32_t y = 0; y < dim.y; ++y) {
			for (std::uint32_t x = 0; x < dim.x; ++x) {
				glm::vec3 n;
				glm::u32vec2 coord = { x, y };

				auto ic = height_map_level.at(coord).r();
				auto iu = ic;
				auto id = ic;
				auto ir = ic;
				auto il = ic;
				if (y + 1 < dim.y)
					iu = height_map_level.at(coord + glm::u32vec2{  0,  1 }).r();
				if (y > 0)
					id = height_map_level.at(coord + glm::u32vec2{  0, -1 }).r();
				if (x + 1 < dim.x)
					ir = height_map_level.at(coord + glm::u32vec2{  1,  0 }).r();
				if (x > 0)
					il = height_map_level.at(coord + glm::u32vec2{ -1,  0 }).r();

				const float c = static_cast<float>(ic);
				const float u = static_cast<float>(iu);
				const float d = static_cast<float>(id);
				const float l = static_cast<float>(il);
				const float r = static_cast<float>(ir);

				// Compute normal
				n.z = 1.0f;
				n.y = ((c - d) * .5f + (u - c) * .5f) * height_scale;
				n.x = ((c - r) * .5f + (l - c) * .5f) * height_scale;
				n = glm::normalize(n);

				// Write
				nm_level.at(coord).r() = static_cast<T>(n.x);
				nm_level.at(coord).g() = static_cast<T>(n.y);
				nm_level.at(coord).b() = static_cast<T>(n.z);
				if constexpr (has_height_in_alpha_channel) {
					nm_level.at(coord).a() = static_cast<T>(c);
				}
			}
		}

		return std::move(nm);
	}
};

}
}
