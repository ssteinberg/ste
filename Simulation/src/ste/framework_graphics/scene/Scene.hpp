// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gpu_dispatchable.hpp"

#include "resource_instance.hpp"
#include "resource_loading_task.hpp"
#include "glsl_program_loading_task.hpp"

#include "SceneProperties.hpp"
#include "deferred_gbuffer.hpp"
#include "object_group_indirect_command_buffer.hpp"

#include "AtomicCounterBufferObject.hpp"
#include "ShaderStorageBuffer.hpp"

#include "ObjectGroup.hpp"
#include "light_storage.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class Scene : public gpu_dispatchable {
	using Base = gpu_dispatchable;

	friend class Resource::resource_loading_task<Scene>;

private:
	static constexpr int shadow_proj_id_to_ll_id_table_size = max_active_lights_per_frame;

	struct shadow_projection_instance_to_ll_idx_translation {
		std::uint16_t ll_idx[shadow_proj_id_to_ll_id_table_size];
	};

	static constexpr Core::BufferUsage::buffer_usage usage = static_cast<Core::BufferUsage::buffer_usage>(Core::BufferUsage::BufferUsageSparse);
	static constexpr int pages = 8192;

 	using sproj_id_to_llid_tt_buffer_type = Core::ShaderStorageBuffer<shadow_projection_instance_to_ll_idx_translation, usage>;

private:
	ObjectGroup objects;
	SceneProperties scene_props;
	const deferred_gbuffer *gbuffer{ nullptr };

	mutable Core::AtomicCounterBufferObject<> culled_objects_counter;
	mutable object_group_indirect_command_buffer idb;
	mutable object_group_indirect_command_buffer shadow_idb;
	mutable sproj_id_to_llid_tt_buffer_type sproj_id_to_llid_tt;

	Resource::resource_instance<Core::glsl_program> object_program;

public:
	Scene(const StEngineControl &ctx);
	~Scene() noexcept {}

	void update_scene() {
		objects.update_dirty_buffers();
		scene_props.lights_storage().update();
		scene_props.materials_storage().update();
	}

	SceneProperties &scene_properties() { return scene_props; }
	const SceneProperties &scene_properties() const { return scene_props; }

	ObjectGroup &object_group() { return objects; }
	const ObjectGroup &object_group() const { return objects; }

	void draw_object_group() const;

	void set_target_gbuffer(const deferred_gbuffer *gbuffer) { this->gbuffer = gbuffer; }
	void bind_buffers() const;

	void clear_indirect_command_buffers() const {
		std::uint32_t zero = 0;
		idb.buffer().clear(gli::format::FORMAT_R32_UINT_PACK32, &zero, 0, objects.get_draw_buffers().size());
		shadow_idb.buffer().clear(gli::format::FORMAT_R32_UINT_PACK32, &zero, 0, objects.get_draw_buffers().size());
		culled_objects_counter.clear(gli::format::FORMAT_R32_UINT_PACK32, &zero);
	}

	auto &get_idb() const { return idb; }
	auto &get_shadow_idb() const { return shadow_idb; }
	auto &get_culled_objects_counter() const { return culled_objects_counter; }
	auto &get_sproj_id_to_llid_tt() const { return sproj_id_to_llid_tt; }

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}

namespace Resource {

template <>
class resource_loading_task<Graphics::Scene> {
	using R = Graphics::Scene;

public:
	template <typename ... Ts>
	auto loader(const StEngineControl &ctx, Ts&&... args) {
		return ctx.scheduler().schedule_now([=, &ctx]() {
			auto object = std::make_unique<R>(ctx, std::forward<Ts>(args)...);

			object->object_program.wait();

			return object;
		});
	}
};

}
}
