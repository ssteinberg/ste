// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace graphics {

constexpr std::uint32_t light_type_directional_bit = 1 << 0;
constexpr std::uint32_t light_type_shape_bit = 1 << 1;
constexpr std::uint32_t light_type_two_sided_bit = 1 << 2;
constexpr std::uint32_t light_type_textured_bit = 1 << 3;

constexpr std::uint32_t light_shape_sphere = (0 & 7) << 4;
constexpr std::uint32_t light_shape_quad = (1 & 7) << 4;
constexpr std::uint32_t light_shape_polygon = (2 & 7) << 4;
constexpr std::uint32_t light_shape_convex_polyhedron = (3 & 7) << 4;

enum class light_type : std::uint32_t {
	Point = 0x0,
	Direction = light_type_directional_bit,

	// Shape lights
	Sphere = light_type_shape_bit | light_shape_sphere | light_type_two_sided_bit,
	QuadOnesided = light_type_shape_bit | light_shape_quad,
	QuadTwosided = light_type_shape_bit | light_shape_quad | light_type_two_sided_bit,
	QuadTexturedOnesided = light_type_shape_bit | light_shape_quad | light_type_textured_bit,
	QuadTexturedTwosided = light_type_shape_bit | light_shape_quad | light_type_textured_bit | light_type_two_sided_bit,
	PolygonOnesided = light_type_shape_bit | light_shape_polygon,
	PolygonTwosided = light_type_shape_bit | light_shape_polygon | light_type_two_sided_bit,
	ConvexPolyhedron = light_type_shape_bit | light_shape_convex_polyhedron | light_type_two_sided_bit,
};

}
}
