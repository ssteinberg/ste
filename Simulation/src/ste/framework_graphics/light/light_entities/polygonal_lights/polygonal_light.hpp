// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <shaped_light.hpp>

#include <vector>
#include <array>

namespace ste {
namespace graphics {

class polygonal_light : public shaped_light {
	using Base = shaped_light;

protected:
	polygonal_light(LightType type, 
					const rgb &color,
					float intensity,
					const glm::vec3 &position, 
					shaped_light_points_storage_info storage_info) : shaped_light(type,
																				  color, 
																				  intensity,
																				  position,
																				  storage_info) {
		assert((static_cast<std::uint32_t>(type) & light_shape_polygon) == light_shape_polygon
			   && "Type is not a polygonal light!");
	}

	static float calculate_area(const glm::vec3 *points, std::size_t size, const glm::vec3 &n) {
		glm::vec3 res = { 0,0,0 };
		for (unsigned i=0; i<size; ++i) {
			glm::vec3 v0 = points[i];
			glm::vec3 v1 = points[i + 1 == size ? 0 : i + 1];
			res += glm::cross(v0, v1);
		}

		return .5f * glm::abs(glm::dot(n, res));
	}

public:
	virtual ~polygonal_light() noexcept {}

	void set_points(const glm::vec3 *points, std::size_t size) {
		glm::vec3 n = { 0,0,0 };
		float area = .0f;
		if (size > 2) {
			n = glm::normalize(glm::cross(points[1] - points[0], points[2] - points[0]));
			area = calculate_area(points, size, n);
		}
		Base::set_points(points, size, area, n);
	}
	void set_points(const std::vector<glm::vec3> &points) { set_points(&points[0], points.size()); }
	template <int N>
	void set_points(const std::array<glm::vec3, N> &points) { set_points(&points[0], points.size()); }
};

class polygonal_light_onesided : public polygonal_light {
public:
	polygonal_light_onesided(const rgb &color,
							 float intensity,
							 const glm::vec3 &position,
							 shaped_light_points_storage_info storage_info) : polygonal_light(LightType::PolygonOnesided,
																							  color, intensity, position,
																							  storage_info) {}
	virtual ~polygonal_light_onesided() noexcept {}
};

class polygonal_light_twosided : public polygonal_light {
public:
	polygonal_light_twosided(const rgb &color,
							 float intensity,
							 const glm::vec3 &position,
							 shaped_light_points_storage_info storage_info) : polygonal_light(LightType::PolygonTwosided,
																							  color, intensity, position,
																							  storage_info) {}
	virtual ~polygonal_light_twosided() noexcept {}
};

}
}
