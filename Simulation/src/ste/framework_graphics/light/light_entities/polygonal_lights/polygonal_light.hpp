// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include "stdafx.hpp"
#include "shaped_light.hpp"

#include <vector>
#include <array>

namespace StE {
namespace Graphics {

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

public:
	virtual ~polygonal_light() noexcept {}

	using Base::set_points;
	void set_points(const std::vector<glm::vec3> &points) { Base::set_points(&points[0], points.size()); }
	template <int N>
	void set_points(const std::array<glm::vec3, N> &points) { Base::set_points(&points[0], points.size()); }
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
