// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <shaped_light.hpp>

#include <lib/vector.hpp>
#include <array>

namespace ste {
namespace graphics {

class convex_polyhedron_light : public shaped_light {
	using Base = shaped_light;

protected:
	convex_polyhedron_light(const rgb &color,
							cd_t intensity,
							const metre_vec3 &position,
							shaped_light_points_storage_info storage_info) : shaped_light(light_type::ConvexPolyhedron,
																						  color,
																						  intensity,
																						  position,
																						  storage_info) {}

public:
	virtual ~convex_polyhedron_light() noexcept {}

	void set_points(const ste_context &ctx,
					gl::command_recorder &recorder,
					const metre_vec3 *points, std::size_t size) { Base::set_points(ctx,
																				   recorder,
																				   points, size, 0_m², {0,0,0}); }
	void set_points(const ste_context &ctx,
					gl::command_recorder &recorder,
					const lib::vector<metre_vec3> &points) { set_points(ctx,
																		recorder,
																		&points[0], points.size()); }
	template <int N>
	void set_points(const ste_context &ctx,
					gl::command_recorder &recorder,
					const std::array<metre_vec3, N> &points) { set_points(ctx,
																		  recorder,
																		  &points[0], points.size()); }
};

}
}
