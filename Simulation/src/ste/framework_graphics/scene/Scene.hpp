// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"
#include "gpu_dispatchable.hpp"

#include "SceneProperties.hpp"
#include "deferred_gbuffer.hpp"
#include "object_group_indirect_command_buffer.hpp"

#include "AtomicCounterBufferObject.hpp"

#include "ObjectGroup.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class Scene : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	ObjectGroup objects;
	SceneProperties scene_props;
	const deferred_gbuffer *gbuffer{ nullptr };

	mutable Core::AtomicCounterBufferObject<> culled_objects_counter;
	mutable object_group_indirect_command_buffer idb;
	mutable object_group_indirect_command_buffer shadow_idb;

	std::shared_ptr<Core::GLSLProgram> object_program;

public:
	Scene(const StEngineControl &ctx);
	~Scene() noexcept {}

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

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
