// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <shaped_light.hpp>

#include <command_recorder.hpp>

#include <lib/vector.hpp>
#include <array>

namespace ste {
namespace graphics {

class polygonal_light : public shaped_light {
	using Base = shaped_light;

protected:
	polygonal_light(light_type type, 
					const rgb &color,
					cd_t intensity,
					const metre_vec3 &position,
					shaped_light_points_storage_info storage_info) : shaped_light(type,
																				  color, 
																				  intensity,
																				  position,
																				  storage_info) {
		assert((static_cast<std::uint32_t>(type) & light_shape_polygon) == light_shape_polygon
			   && "Type is not a polygonal light!");
	}

	static auto calculate_area(const metre_vec3 *points, std::size_t size, const glm::vec3 &n) {
		glm::vec3 res = { 0, 0, 0 };
		for (unsigned i=0; i<size; ++i) {
			const auto &v0 = points[i];
			const auto &v1 = points[(i + 1) % size];
			res += glm::cross(v0.v(), v1.v());
		}

		return .5_m² * glm::abs(glm::dot(n, res));
	}

public:
	virtual ~polygonal_light() noexcept {}

	void set_points(const ste_context &ctx,
					gl::command_recorder &recorder,
					const metre_vec3 *points, std::size_t size) {
		glm::vec3 n = { 0,0,0 };
		auto area = .0_m²;
		if (size > 2) {
			n = glm::normalize(glm::cross((points[1] - points[0]).v(), 
										  (points[2] - points[0]).v()));
			area = calculate_area(points, size, n);
		}
		Base::set_points(ctx,
						 recorder,
						 points, size, area, n);
	}
	void set_points(const ste_context &ctx,
					gl::command_recorder &recorder,
					const lib::vector<metre_vec3> &points) {
		set_points(ctx,
				   recorder,
				   &points[0], points.size());
	}
	template <int N>
	void set_points(const ste_context &ctx,
					gl::command_recorder &recorder,
					const std::array<metre_vec3, N> &points) {
		set_points(ctx,
				   recorder,
				   &points[0], points.size());
	}
};

class polygonal_light_onesided : public polygonal_light {
public:
	polygonal_light_onesided(const rgb &color,
							 cd_t intensity,
							 const metre_vec3 &position,
							 shaped_light_points_storage_info storage_info) : polygonal_light(light_type::PolygonOnesided,
																							  color, intensity, position,
																							  storage_info) {}
	virtual ~polygonal_light_onesided() noexcept {}
};

class polygonal_light_twosided : public polygonal_light {
public:
	polygonal_light_twosided(const rgb &color,
							 cd_t intensity,
							 const metre_vec3 &position,
							 shaped_light_points_storage_info storage_info) : polygonal_light(light_type::PolygonTwosided,
																							  color, intensity, position,
																							  storage_info) {}
	virtual ~polygonal_light_twosided() noexcept {}
};

}
}
