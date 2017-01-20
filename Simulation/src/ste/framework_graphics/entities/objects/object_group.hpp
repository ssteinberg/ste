// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "entity.hpp"
#include "object.hpp"
#include "gpu_dispatchable.hpp"

#include "object_group_draw_buffers.hpp"

#include <unordered_map>
#include <memory>

namespace StE {
namespace Graphics {

class object_group : public entity_affine {
	using Base = gpu_dispatchable;

private:
	using signal_connection_type = object::signal_type::connection_type;

	struct object_information {
		std::size_t index;
		std::shared_ptr<signal_connection_type> connection;
	};

	using objects_map_type = std::unordered_map<std::shared_ptr<object>, object_information>;

private:
	object_group_draw_buffers draw_buffers;
	objects_map_type objects;

	std::size_t total_vertices{ 0 };
	std::size_t total_indices{ 0 };

	mutable std::vector<object*> signalled_objects;

public:
	object_group() {}
	~object_group() noexcept;

	void add_object(const std::shared_ptr<object> &);
	void remove_all();

	auto& get_draw_buffers() const { return draw_buffers; }

	void update_dirty_buffers() const;

	std::size_t total_objects() const { return objects.size(); }
};

}
}
