// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <light.hpp>

#include <stable_vector.hpp>
#include <command_recorder.hpp>

namespace ste {
namespace graphics {

class shaped_light : public light {
	using Base = light;

public:
	using shaped_light_point_type = glm::uvec2;
	struct shaped_light_points_storage_info {
		gl::stable_vector<shaped_light_point_type> *storage;
	};

protected:
	shaped_light_points_storage_info storage_info;

protected:
	shaped_light(light_type type,
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

	void set_points(gl::command_recorder &recorder,
					const glm::vec3 *points, 
					std::size_t size, 
					float surface_area, 
					const glm::vec3 &n) {
		lib::vector<shaped_light_point_type> points_copy;
		float r = .0f;

		if (size > 0) {
			points_copy.reserve(size);

			// Center points and compute radius
			glm::vec3 center = { 0,0,0 };
			for (auto *p = points; p != points + size; ++p)
				center += glm::vec3{ p->x, p->y, p->z } / static_cast<float>(size);
			for (auto *p = points; p != points + size; ++p)
				r = glm::max(r, glm::length(*p - center));

			for (auto *p = points; p != points + size; ++p) {
				auto t = *p - center;

				// Encode
				points_copy.push_back({ glm::packHalf2x16({t.x, t.y}),
									    glm::packHalf2x16({t.z, .0f}) });
			}
		}

		// Add to buffer
		std::uint64_t idx = descriptor.get_polygonal_light_buffer_offset();
		auto current_count = static_cast<std::size_t>(this->get_points_count());
		if (current_count > 0) {
			if (current_count < size) {
				// Erase old points and insert new
				storage_info.storage->tombstone(idx, current_count);
				recorder << storage_info.storage->insert_cmd(points_copy, idx);
			}
			else {
				// Overwrite
				if (size > 0)
					recorder << storage_info.storage->overwrite_cmd(idx, points_copy);

				// Erase tail
				if (current_count > size)
					storage_info.storage->tombstone(idx + size, current_count - size);
			}
		}
		else {
			recorder << storage_info.storage->insert_cmd(points_copy, idx);
		}

		float sqrt_surface_area = glm::sqrt(surface_area);

		// Update descriptor
		descriptor.set_polygonal_light_points(static_cast<std::uint8_t>(size), 
											  static_cast<std::uint32_t>(idx));
		descriptor.radius = r;
		update_effective_range(sqrt_surface_area);
		Base::notify();
	}

public:
	virtual ~shaped_light() noexcept {
		auto idx = descriptor.get_polygonal_light_buffer_offset();
		auto count = this->get_points_count();
		if (count > 0)
			storage_info.storage->tombstone(idx, count);
	}

	std::uint32_t get_points_count() const { return descriptor.get_polygonal_light_point_count(); }

	void set_position(const glm::vec3 &p) {
		descriptor.position = decltype(descriptor.position){ p.x, p.y, p.z };
		Base::notify();
	}
	glm::vec3 get_position() const override { return{ descriptor.position.x, descriptor.position.y, descriptor.position.z }; }
};

}
}
