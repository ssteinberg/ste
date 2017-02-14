// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_engine_control.hpp>
#include <gpu_dispatchable.hpp>

#include <scene_properties.hpp>
#include <deferred_gbuffer.hpp>
#include <object_group_indirect_command_buffer.hpp>

#include <glsl_program.hpp>
#include <atomic_counter_buffer_object.hpp>
#include <shader_storage_buffer.hpp>

#include <object_group.hpp>
#include <light_storage.hpp>

#include <memory>

namespace StE {
namespace Graphics {

class scene : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	static constexpr int shadow_pltt_size = max_active_lights_per_frame;
	static constexpr int directional_shadow_pltt_size = max_active_directional_lights_per_frame;

	template <int tt_slots>
	struct shadow_drawid_to_lightid_ttl {
		std::uint32_t entries[tt_slots];
	};

	static constexpr Core::BufferUsage::buffer_usage usage = static_cast<Core::BufferUsage::buffer_usage>(Core::BufferUsage::BufferUsageSparse);
	static constexpr int pages = 8192;

	template <int tt_slots>
	class shadow_projection_data {
		using table = shadow_drawid_to_lightid_ttl<tt_slots>;
		using table_buffer_type = Core::shader_storage_buffer<table, usage>;

	public:
		shadow_projection_data() : proj_id_to_light_id_translation_table(pages * std::max<std::size_t>(65536, table_buffer_type::page_size()) / sizeof(table)) {}

		object_group_indirect_command_buffer idb;
		table_buffer_type proj_id_to_light_id_translation_table;

		void commit_range(std::size_t start, std::size_t end) {
			idb.buffer().commit_range(start, end);
			proj_id_to_light_id_translation_table.commit_range(start, end);
		}
	};

private:
	object_group objects;
	scene_properties scene_props;
	const deferred_gbuffer *gbuffer{ nullptr };

	mutable Core::atomic_counter_buffer_object<> culled_objects_counter;
	mutable object_group_indirect_command_buffer idb;

	mutable shadow_projection_data<shadow_pltt_size> shadow_projection;
	mutable shadow_projection_data<directional_shadow_pltt_size> directional_shadow_projection;

	Resource::resource_instance<Resource::glsl_program> object_program;

public:
	scene(const ste_engine_control &ctx);
	~scene() noexcept {}

	void update_scene() {
		objects.update_dirty_buffers();
		scene_props.update();
	}

	scene_properties &properties() { return scene_props; }
	const scene_properties &properties() const { return scene_props; }

	object_group &get_object_group() { return objects; }
	const object_group &get_object_group() const { return objects; }

	void draw_object_group() const;

	void set_target_gbuffer(const deferred_gbuffer *gbuffer) { this->gbuffer = gbuffer; }
	void bind_buffers() const;

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

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
