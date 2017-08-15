// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <scene_properties.hpp>
#include <deferred_gbuffer.hpp>
#include <object_group_indirect_command_buffer.hpp>

#include <object_group.hpp>
#include <light_storage.hpp>

#include <lib/unique_ptr.hpp>

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
			recorder << idb.buffer().resize_cmd(size);
			recorder << proj_id_to_light_id_translation_table.resize_cmd(size);
		}
	};

private:
	object_group objects;
	scene_properties scene_props;
//	const deferred_gbuffer *gbuffer{ nullptr };

	mutable gl::array<std::uint32_t> culled_objects_counter;
	mutable object_group_indirect_command_buffer idb;

	mutable shadow_projection_data<shadow_pltt_size> shadow_projection;
	mutable shadow_projection_data<directional_shadow_pltt_size> directional_shadow_projection;

public:
	scene(const ste_context &ctx);
	~scene() noexcept {}

	void update_scene(gl::command_recorder &recorder) {
		objects.update_dirty_buffers();
		scene_props.update(recorder);
	}

	scene_properties &properties() { return scene_props; }
	const scene_properties &properties() const { return scene_props; }

	object_group &get_object_group() { return objects; }
	const object_group &get_object_group() const { return objects; }

//	void set_target_gbuffer(const deferred_gbuffer *gbuffer) { this->gbuffer = gbuffer; }

	void clear_indirect_command_buffers() const {
		std::uint32_t zero = 0;
		idb.buffer().clear(gli::format::FORMAT_R32_UINT_PACK32, &zero, 0, objects.get_draw_buffers().size());
		shadow_projection.idb.buffer().clear(gli::format::FORMAT_R32_UINT_PACK32, &zero, 0, objects.get_draw_buffers().size());
		directional_shadow_projection.idb.buffer().clear(gli::format::FORMAT_R32_UINT_PACK32, &zero, 0, objects.get_draw_buffers().size());
		culled_objects_counter.clear(gli::format::FORMAT_R32_UINT_PACK32, &zero);
	}

	auto &get_idb() const { return idb; }
	auto &get_culled_objects_counter() const { return culled_objects_counter; }

	auto &get_shadow_projection_buffers() const { return shadow_projection; }
	auto &get_directional_shadow_projection_buffers() const { return directional_shadow_projection; }
};

}
}
