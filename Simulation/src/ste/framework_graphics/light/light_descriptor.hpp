// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <light_type.hpp>

#include <std430.hpp>

namespace ste {
namespace graphics {

struct light_descriptor {
	using buffer_data = gl::std430<glm::vec4, glm::vec4, std::uint32_t, std::uint32_t, float, std::uint32_t, glm::vec4>;

	glm::vec3		position;		float radius{ .0f };
	glm::vec3		emittance;		light_type type;

	std::uint32_t texture_idx;
	std::uint32_t _unused;
	float			effective_range_or_directional_distance{ .0f };
	std::uint32_t	polygonal_light_points_and_offset_or_cascade_idx{ 0 };

	float _internal[4];

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
		return buffer_data(std::make_tuple(glm::vec4(position, radius), 
										   glm::vec4(emittance, glm::uintBitsToFloat(static_cast<std::uint32_t>(type))), 
										   texture_idx, 
										   0u, 
										   effective_range_or_directional_distance, 
										   polygonal_light_points_and_offset_or_cascade_idx, 
										   glm::vec4{ .0f }));
	}
};

}
}
