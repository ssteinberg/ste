// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include "stdafx.hpp"
#include "light_type.hpp"

namespace StE {
namespace Graphics {

struct light_descriptor {
	glm::vec3 position;		float radius{ .0f };
	glm::vec3 emittance;	std::uint32_t polygonal_light_points_and_offset{ 0 };

	LightType type;

	float directional_distance;

	std::uint32_t cascade_idx{ 0xFFFFFFFF };

	float _internal[5];

public:
	void set_polygonal_light_points(std::uint8_t points, std::uint32_t offset) {
		polygonal_light_points_and_offset = (points << 24) | (offset & 0x00FFFFFF);
	}

	auto get_polygonal_light_buffer_offset() const {
		return polygonal_light_points_and_offset & 0x00FFFFFF;
	}

	auto get_polygonal_light_point_count() const {
		return polygonal_light_points_and_offset >> 24;
	}
};

}
}
