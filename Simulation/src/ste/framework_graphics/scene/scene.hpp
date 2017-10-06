// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <scene_properties.hpp>
#include <object_group_indirect_command_buffer.hpp>

#include <object_group.hpp>
#include <light_storage.hpp>

#include <vector.hpp>
#include <command_recorder.hpp>

namespace ste {
namespace graphics {

class scene {
private:
	static constexpr int shadow_pltt_size = max_active_lights_per_frame;
	static constexpr int directional_shadow_pltt_size = max_active_directional_lights_per_frame;

private:
	object_group objects;
	scene_properties scene_props;

	object_group_indirect_command_buffer idb;

public:
	scene(const ste_context &ctx)
		: objects(ctx),
		scene_props(ctx),
		idb(ctx,
			gl::buffer_usage::storage_buffer,
			"indirect draw buffer")
	{}
	~scene() noexcept {}

	void update_scene(gl::command_recorder &recorder) {
		objects.update_dirty_buffers(recorder);
		scene_props.update(recorder);
	}

	scene_properties &properties() { return scene_props; }
	const scene_properties &properties() const { return scene_props; }

	object_group &get_object_group() { return objects; }
	const object_group &get_object_group() const { return objects; }

	auto &get_idb() const { return idb; }

	void resize_indirect_command_buffers(const ste_context &ctx,
										 gl::command_recorder &recorder,
										 std::size_t size) {
		recorder << idb->resize_cmd(ctx,
									size);
	}
};

}
}
