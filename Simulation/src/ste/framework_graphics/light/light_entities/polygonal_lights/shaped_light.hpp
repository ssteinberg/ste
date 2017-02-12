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
	using shaped_light_point_type = glm::uvec2;
	struct shaped_light_points_storage_info {
		Core::gstack_stable<shaped_light_point_type> *storage;
	};

protected:
	shaped_light_points_storage_info storage_info;

private:
	static glm::vec2 _norm_oct_wrap(const glm::vec2 &v) {
		return (glm::vec2(1.f) - abs(glm::vec2{ v.y, v.x })) * glm::vec2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0);
	}
	static glm::vec2 norm3x32_to_snorm2x32(glm::vec3 n) {
		n /= (abs(n.x) + abs(n.y) + abs(n.z));
		glm::vec2 xy = n.z >= 0.0 ? glm::vec2{ n.x, n.y } : _norm_oct_wrap(glm::vec2{ n.x, n.y });
		return xy;
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

	void set_points(const glm::vec3 *points, std::size_t size, float surface_area, const glm::vec3 &n) {
		std::vector<shaped_light_point_type> points_copy;
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
		auto idx = descriptor.get_polygonal_light_buffer_offset();
		auto current_count = this->get_points_count();
		if (current_count > 0) {
			if (current_count < size) {
				// Erase old points and insert new
				storage_info.storage->mark_tombstone(idx, current_count);
				idx = storage_info.storage->insert(points_copy);
			}
			else {
				// Overwrite
				if (size > 0)
					storage_info.storage->overwrite(idx, points_copy);

				// Erase tail
				if (current_count > size)
					storage_info.storage->mark_tombstone(idx + size, current_count - size);
			}
		}
		else {
			idx = storage_info.storage->insert(points_copy);
		}

		float sqrt_surface_area = glm::sqrt(surface_area);

		// Update descriptor
		descriptor.set_polygonal_light_points(size, idx);
		descriptor.radius = r;
		descriptor.normal_pack = glm::packSnorm2x16(norm3x32_to_snorm2x32(n));
		update_effective_range(sqrt_surface_area);
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
