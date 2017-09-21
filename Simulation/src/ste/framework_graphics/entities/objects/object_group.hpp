// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <entity.hpp>
#include <object.hpp>
#include <object_group_draw_buffers.hpp>

#include <command_recorder.hpp>

#include <lib/unordered_map.hpp>
#include <mutex>

namespace ste {
namespace graphics {

class object_group : public entity_affine {
private:
	using signal_connection_type = object::model_change_signal_type::connection_type;

	struct object_information {
		std::size_t index;
		object::model_change_signal_type::connection_type connection;
	};

	using objects_map_type = lib::unordered_map<lib::shared_ptr<object>, object_information>;

private:
	object_group_draw_buffers draw_buffers;
	objects_map_type objects;

	mutable lib::vector<object*> signalled_objects;
	mutable std::mutex m;

public:
	object_group(const ste_context &ctx) : draw_buffers(ctx) {}
	~object_group() noexcept;

	void add_object(const ste_context &ctx,
					gl::command_recorder &recorder,
					const lib::shared_ptr<object> &);
	void remove_all();
	void update_dirty_buffers(gl::command_recorder &recorder) const;

	auto& get_draw_buffers() { return draw_buffers; }
	auto& get_draw_buffers() const { return draw_buffers; }

	auto draw_count() const { return objects.size(); }
};

}
}
