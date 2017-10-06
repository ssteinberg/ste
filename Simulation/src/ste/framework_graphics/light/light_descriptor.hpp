//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <light_type.hpp>

#include <std430.hpp>

namespace ste {
namespace graphics {

struct light_descriptor {
	using buffer_data = gl::std430<metre_vec3, metre, glm::vec3, std::uint32_t, std::uint32_t, metre, std::uint32_t, float, glm::vec3, float>;

	metre_vec3		position;		metre radius{ 0_m };
	glm::vec3		emittance;		light_type type;

	std::uint32_t	texture_idx;
	metre			effective_range_or_directional_distance{ .0f };
	std::uint32_t	polygonal_light_points_and_offset_or_cascade_idx{ 0 };

	float			_unused0;

	glm::vec3		_internal;
	float			_unused1;

public:
	void set_polygonal_light_points(std::uint8_t points, std::uint32_t offset) {
		polygonal_light_points_and_offset_or_cascade_idx = (points << 24) | (offset & 0x00FFFFFF);
	}

	auto get_polygonal_light_buffer_offset() const {
		return polygonal_light_points_and_offset_or_cascade_idx & 0x00FFFFFF;
	}

	auto get_polygonal_light_point_count() const {
		return polygonal_light_points_and_offset_or_cascade_idx >> 24;
	}

	auto get() const {
		return buffer_data(std::make_tuple(position, radius, 
										   emittance, static_cast<std::uint32_t>(type), 
										   texture_idx, 
										   effective_range_or_directional_distance, 
										   polygonal_light_points_and_offset_or_cascade_idx, 
										   .0f,
										   glm::vec3{ .0f },
										   .0f));
	}
};

}
}
