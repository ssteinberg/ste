//	StE
// © Shlomi Steinberg, 2015-2017
#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <light.hpp>

#include <std430.hpp>

#include <stable_vector.hpp>
#include <command_recorder.hpp>

namespace ste {
namespace graphics {

class shaped_light : public light {
	using Base = light;

public:
	using shaped_light_point_type = gl::std430<glm::u32vec2>;
	struct shaped_light_points_storage_info {
		gl::stable_vector<shaped_light_point_type> *storage;
	};

protected:
	shaped_light_points_storage_info storage_info;

protected:
	shaped_light(light_type type,
				 const rgb &color,
				 cd_t intensity,
				 const metre_vec3 &position,
				 shaped_light_points_storage_info storage_info) : light(color, intensity, 0_m),
		storage_info(storage_info) {
		assert(static_cast<std::uint32_t>(type) & light_type_shape_bit && "Type is not a shaped light!");

		descriptor.type = type;
		descriptor.position = position;
		descriptor.set_polygonal_light_points(0, 0);
	}

	void set_points(const ste_context &ctx,
					gl::command_recorder &recorder,
					const metre_vec3 *points,
					std::size_t size,
					square_metre surface_area,
					const glm::vec3 &n) {
		lib::vector<shaped_light_point_type> points_copy;
		float r = .0f;

		if (size > 0) {
			points_copy.reserve(size);

			// Center points and compute radius
			metre_vec3 center = { 0_m, 0_m, 0_m };
			for (auto *p = points; p != points + size; ++p)
				center += metre_vec3{ p->x, p->y, p->z } / static_cast<float>(size);
			for (auto *p = points; p != points + size; ++p)
				r = glm::max(r, glm::length((*p - center).v()));

			for (auto *p = points; p != points + size; ++p) {
				const auto t = *p - center;

				// Encode
				auto v = glm::u32vec2{ glm::packHalf2x16({ t.x, t.y }), glm::packHalf2x16({ t.z, .0f }) };
				points_copy.push_back(std::make_tuple<glm::u32vec2>(std::move(v)));
			}

            assert(points_copy.size() == size);
		}

		// Add to buffer
		std::uint64_t idx = descriptor.get_polygonal_light_buffer_offset();
		const auto current_count = static_cast<std::size_t>(this->get_points_count());
		if (current_count > 0) {
			if (current_count < size) {
				// Erase old points and insert new
				storage_info.storage->tombstone(idx, current_count);
				recorder << storage_info.storage->insert_cmd(ctx, points_copy, idx);
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
			recorder << storage_info.storage->insert_cmd(ctx, points_copy, idx);
		}

		const float sqrt_surface_area = glm::sqrt(static_cast<float>(surface_area));

		// Update descriptor
		descriptor.set_polygonal_light_points(static_cast<std::uint8_t>(size),
											  static_cast<std::uint32_t>(idx));
		descriptor.radius = metre(r);
		update_effective_range(sqrt_surface_area);
		Base::notify();
	}

public:
	virtual ~shaped_light() noexcept {
		auto idx = descriptor.get_polygonal_light_buffer_offset();
		const auto count = this->get_points_count();
		if (count > 0)
			storage_info.storage->tombstone(idx, count);
	}

	std::uint32_t get_points_count() const { return descriptor.get_polygonal_light_point_count(); }

	void set_position(const metre_vec3 &p) {
		descriptor.position = p;
		Base::notify();
	}
	metre_vec3 get_position() const override { return{ metre(descriptor.position.x), metre(descriptor.position.y), metre(descriptor.position.z) }; }
};

}
}
