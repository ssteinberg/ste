// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include "stdafx.hpp"
#include "light.hpp"

#include "gstack_stable.hpp"

namespace StE {
namespace Graphics {

class shaped_light : public light {
	using Base = light;

public:
	using shaped_light_point_type = glm::vec3;
	struct shaped_light_points_storage_info {
		Core::gstack_stable<shaped_light_point_type> *storage;
	};

protected:
	shaped_light_points_storage_info storage_info;

private:
	static auto points_radius(const glm::vec3 *points, std::size_t size) {
		float r = 0;
		for (auto *p = points; p != points+size; ++p)
			r = glm::max(r, glm::length(*p));
		return r;
	}
	static void center_points(glm::vec3 *points, std::size_t size) {
		glm::vec3 center = { 0,0,0 };
		for (auto *p = points; p != points+size; ++p)
			center += *p;
		center /= size;
		for (auto *p = points; p != points+size; ++p)
			*p -= center;
	}

protected:
	shaped_light(LightType type,
				 const rgb &color,
				 float intensity,
				 const glm::vec3 &position,
				 shaped_light_points_storage_info storage_info) : light(color, intensity, .0f),
																  storage_info(storage_info) {
		assert(static_cast<std::uint32_t>(type) & light_type_shape_bit && "Type is not a shaped light!");

		descriptor.type = type;
		descriptor.position = decltype(descriptor.position){ position.x, position.y, position.z };
		descriptor.set_polygonal_light_points(0, 0);
	}

	void set_points(const glm::vec3 *points, std::size_t size) {
		// Center points
		if (size > 0) {
			std::vector<glm::vec3> points_copy(points, points + size);
			center_points(&points_copy[0], size);
			points = &points_copy[0];
		}

		// Add to buffer
		auto idx = descriptor.get_polygonal_light_buffer_offset();
		auto current_count = this->get_points_count();
		if (current_count > 0) {
			if (current_count < size) {
				// Erase old points and insert new
				storage_info.storage->mark_tombstone(idx, current_count);
				idx = storage_info.storage->insert(points, size);
			}
			else {
				// Overwrite
				if (size > 0)
					storage_info.storage->overwrite(idx, points, size);

				// Erase tail
				if (current_count > size)
					storage_info.storage->mark_tombstone(idx + size, current_count - size);
			}
		}
		else {
			idx = storage_info.storage->insert(points, size);
		}

		// Update descriptor
		descriptor.set_polygonal_light_points(size, idx);
		descriptor.radius = points_radius(points, size);
		Base::notify();
	}

public:
	virtual ~shaped_light() noexcept {
		auto idx = descriptor.get_polygonal_light_buffer_offset();
		auto count = this->get_points_count();
		if (count > 0)
			storage_info.storage->mark_tombstone(idx, count);
	}

	int get_points_count() const { return descriptor.get_polygonal_light_point_count(); }

	void set_position(const glm::vec3 &p) {
		descriptor.position = decltype(descriptor.position){ p.x, p.y, p.z };
		Base::notify();
	}
	glm::vec3 get_position() const override { return{ descriptor.position.x, descriptor.position.y, descriptor.position.z }; }
};

}
}
