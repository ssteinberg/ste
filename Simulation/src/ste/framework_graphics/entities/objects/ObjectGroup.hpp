// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "entity.hpp"
#include "Object.hpp"
#include "gpu_dispatchable.hpp"

#include "object_group_draw_buffers.hpp"

#include "ObjectVertexData.hpp"
#include "Material.hpp"

#include "range.hpp"

#include <unordered_map>
#include <memory>

namespace StE {
namespace Graphics {

class ObjectGroup : public entity_affine {
	using Base = gpu_dispatchable;

private:
	using signal_connection_type = Object::signal_type::connection_type;

	struct object_information {
		std::size_t index;
		std::shared_ptr<signal_connection_type> connection;
	};

	using objects_map_type = std::unordered_map<std::shared_ptr<Object>, object_information>;

private:
	object_group_draw_buffers draw_buffers;

	objects_map_type objects;

	std::size_t total_vertices{ 0 };
	std::size_t total_indices{ 0 };

	mutable std::vector<Object*> signalled_objects;
	mutable std::vector<range<>> ranges_to_lock;

public:
	ObjectGroup() {}
	~ObjectGroup() noexcept;

	void add_object(const std::shared_ptr<Object> &);
	void remove_all();

	void set_model_matrix(const glm::mat4 &m) override {
		entity_affine::set_model_matrix(m);
	}

	auto& get_draw_buffers() const { return draw_buffers; }

	void update_dirty_buffers() const;
	void lock_updated_buffers() const;

	std::size_t total_objects() const { return objects.size(); }
};

}
}
