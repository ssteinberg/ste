// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include "stdafx.hpp"
#include "shaped_light.hpp"

#include <vector>
#include <array>

namespace StE {
namespace Graphics {

class convex_polyhedron_light : public shaped_light {
	using Base = shaped_light;

protected:
	convex_polyhedron_light(const rgb &color,
							float intensity,
							const glm::vec3 &position,
							shaped_light_points_storage_info storage_info) : shaped_light(LightType::ConvexPolyhedron,
																						  color,
																						  intensity,
																						  position,
																						  storage_info) {}

public:
	virtual ~convex_polyhedron_light() noexcept {}

	using Base::set_points;
	void set_points(const std::vector<glm::vec3> &points) { Base::set_points(&points[0], points.size()); }
	template <int N>
	void set_points(const std::array<glm::vec3, N> &points) { Base::set_points(&points[0], points.size()); }
};

}
}
