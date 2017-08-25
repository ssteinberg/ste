// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <shaped_light.hpp>

namespace ste {
namespace graphics {

class quad_light : public shaped_light {
	using Base = shaped_light;

protected:
	quad_light(light_type type,
			   const rgb &color,
			   float intensity,
			   const glm::vec3 &position,
			   shaped_light_points_storage_info storage_info) : shaped_light(type,
																			 color,
																			 intensity,
																			 position,
																			 storage_info) {
		assert((static_cast<std::uint32_t>(type) & light_shape_quad) == light_shape_quad
			   && "Type is not a polygonal light!");
	}

public:
	virtual ~quad_light() noexcept {}

	void set_points(const glm::vec3 *quad_points) {
		glm::vec3 n = glm::cross(quad_points[1] - quad_points[0], quad_points[3] - quad_points[0]);
		float area = glm::length(n);

		Base::set_points(quad_points, 4, area, n / area);
	}
	void set_points(const std::array<glm::vec3, 4> &quad_points) { set_points(&quad_points[0]); }
};

class quad_light_onesided : public quad_light {
public:
	quad_light_onesided(const rgb &color,
						float intensity,
						const glm::vec3 &position,
						shaped_light_points_storage_info storage_info) : quad_light(light_type::QuadOnesided,
																					color, intensity, position,
																					storage_info) {}
	virtual ~quad_light_onesided() noexcept {}
};

class quad_light_twosided : public quad_light {
public:
	quad_light_twosided(const rgb &color,
						float intensity,
						const glm::vec3 &position,
						shaped_light_points_storage_info storage_info) : quad_light(light_type::QuadTwosided,
																					color, intensity, position,
																					storage_info) {}
	virtual ~quad_light_twosided() noexcept {}
};

}
}
