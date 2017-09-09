// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <scene_properties.hpp>
#include <object_group_indirect_command_buffer.hpp>

#include <object_group.hpp>
#include <light_storage.hpp>

#include <command_recorder.hpp>
#include <cmd_fill_buffer.hpp>

namespace ste {
namespace graphics {

class scene {
private:
	static constexpr int shadow_pltt_size = max_active_lights_per_frame;
	static constexpr int directional_shadow_pltt_size = max_active_directional_lights_per_frame;

	template <int tt_slots>
	struct shadow_drawid_to_lightid_ttl {
		std::uint32_t entries[tt_slots];
	};

	template <int tt_slots>
	class shadow_projection_data {
		using table = shadow_drawid_to_lightid_ttl<tt_slots>;
		using table_buffer_type = gl::vector<table>;

	public:
		shadow_projection_data(const ste_context &ctx)
			: idb(ctx, gl::buffer_usage::storage_buffer),
			proj_id_to_light_id_translation_table(ctx, gl::buffer_usage::storage_buffer) {}

		object_group_indirect_command_buffer idb;
		table_buffer_type proj_id_to_light_id_translation_table;

		void resize(gl::command_recorder &recorder, std::size_t size) {
			recorder << idb.get().resize_cmd(size);
			recorder << proj_id_to_light_id_translation_table.resize_cmd(size);
		}
	};

private:
	object_group objects;
	scene_properties scene_props;

	mutable gl::array<std::uint32_t> culled_objects_counter;
	mutable object_group_indirect_command_buffer idb;

	mutable shadow_projection_data<shadow_pltt_size> shadow_projection;
	mutable shadow_projection_data<directional_shadow_pltt_size> directional_shadow_projection;

public:
	scene(const ste_context &ctx)
		: objects(ctx),
		scene_props(ctx),
		culled_objects_counter(ctx, 1, gl::buffer_usage::storage_buffer),
		idb(ctx),
		shadow_projection(ctx),
		directional_shadow_projection(ctx)
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
	auto &get_culled_objects_counter() const { return culled_objects_counter; }

	auto &get_shadow_projection_buffers() const { return shadow_projection; }
	auto &get_directional_shadow_projection_buffers() const { return directional_shadow_projection; }

	void resize_indirect_command_buffers(gl::command_recorder &recorder,
										 std::size_t size) {
		recorder << idb->resize_cmd(size);
		shadow_projection.resize(recorder, size);
		directional_shadow_projection.resize(recorder, size);
	}
	void clear_indirect_command_buffers(gl::command_recorder &recorder) const {
		recorder
			<< gl::cmd_fill_buffer(static_cast<gl::device_buffer_base&>(idb.get().get()), 0u)
			<< gl::cmd_fill_buffer(static_cast<gl::device_buffer_base&>(shadow_projection.idb.get().get()), 0u)
			<< gl::cmd_fill_buffer(static_cast<gl::device_buffer_base&>(directional_shadow_projection.idb.get().get()), 0u)
			<< gl::cmd_fill_buffer(culled_objects_counter.get(), 0u);
	}
};

}
}
