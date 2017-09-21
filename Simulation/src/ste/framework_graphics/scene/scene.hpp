// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <scene_properties.hpp>
#include <object_group_indirect_command_buffer.hpp>

#include <object_group.hpp>
#include <light_storage.hpp>

#include <vector.hpp>
#include <std430.hpp>

#include <command_recorder.hpp>

namespace ste {
namespace graphics {

class scene {
private:
	static constexpr int shadow_pltt_size = max_active_lights_per_frame;
	static constexpr int directional_shadow_pltt_size = max_active_directional_lights_per_frame;

//	template <int tt_slots>
//	using shadow_drawid_to_lightid_ttl = gl::std430<std::uint32_t[tt_slots]>;

//	template <int tt_slots>
//	class shadow_projection_data {
//		using table_buffer_type = gl::vector<shadow_drawid_to_lightid_ttl<tt_slots>>;
//
//	public:
//		shadow_projection_data(const ste_context &ctx)
//			: idb(ctx,
//				  gl::buffer_usage::storage_buffer,
//				  "shadow_projection_data idb"),
//			proj_id_to_light_id_translation_table(ctx,
//												  gl::buffer_usage::storage_buffer,
//												  "shadow_projection_data proj_id_to_light_id_translation_table") {}
//
//		object_group_indirect_command_buffer idb;
//		table_buffer_type proj_id_to_light_id_translation_table;
//
//		void resize(const ste_context &ctx,
//					gl::command_recorder &recorder, 
//					std::size_t size) {
//			recorder << idb.get().resize_cmd(ctx, size);
//			recorder << proj_id_to_light_id_translation_table.resize_cmd(ctx, size);
//		}
//	};

private:
	object_group objects;
	scene_properties scene_props;

	mutable object_group_indirect_command_buffer idb;

//	mutable shadow_projection_data<shadow_pltt_size> shadow_projection;
//	mutable shadow_projection_data<directional_shadow_pltt_size> directional_shadow_projection;

public:
	scene(const ste_context &ctx)
		: objects(ctx),
		scene_props(ctx),
		idb(ctx,
			gl::buffer_usage::storage_buffer,
			"indirect draw buffer")
//		shadow_projection(ctx),
//		directional_shadow_projection(ctx)
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

//	auto &get_shadow_projection_buffers() const { return shadow_projection; }
//	auto &get_directional_shadow_projection_buffers() const { return directional_shadow_projection; }

	void resize_indirect_command_buffers(const ste_context &ctx,
										 gl::command_recorder &recorder,
										 std::size_t size) {
		recorder << idb->resize_cmd(ctx,
									size);

//		shadow_projection.resize(ctx, recorder, size);
//		directional_shadow_projection.resize(ctx, recorder, size);
	}
};

}
}
