// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "entity.hpp"
#include "Object.hpp"
#include "gpu_dispatchable.hpp"

#include "gl_current_context.hpp"
#include "GLSLProgram.hpp"

#include "deferred_gbuffer.hpp"
#include "object_group_draw_buffers.hpp"

#include "ObjectVertexData.hpp"
#include "Material.hpp"
#include "SceneProperties.hpp"

#include "range.hpp"

#include <unordered_map>
#include <memory>

namespace StE {
namespace Graphics {

class ObjectGroup : public gpu_dispatchable, public entity_affine {
	using Base = gpu_dispatchable;

private:
	using signal_connection_type = Object::signal_type::connection_type;

	struct object_information {
		std::size_t index;
		std::shared_ptr<signal_connection_type> connection;
	};

	using objects_map_type = std::unordered_map<std::shared_ptr<Object>, object_information>;

	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;

private:
	object_group_draw_buffers draw_buffers;

	const SceneProperties *scene_props;
	const deferred_gbuffer *gbuffer{ nullptr };
	objects_map_type objects;

	std::size_t total_vertices{ 0 };
	std::size_t total_indices{ 0 };

	mutable std::vector<Object*> signalled_objects;
	mutable std::vector<range<>> ranges_to_lock;

	std::shared_ptr<Core::GLSLProgram> object_program;

private:
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

public:
	ObjectGroup(const StEngineControl &ctx, const SceneProperties *props);
	~ObjectGroup() noexcept;

	void set_target_gbuffer(const deferred_gbuffer *gbuffer) { this->gbuffer = gbuffer; }

	void bind_buffers() const;
	void draw_object_group() const { dispatch(); }

	void add_object(const std::shared_ptr<Object> &);
	void remove_all();

	void set_model_matrix(const glm::mat4 &m) override {
		entity_affine::set_model_matrix(m);
	}

	auto& get_draw_buffers() const { return draw_buffers; }

	void update_dirty_buffers() const;
	void lock_updated_buffers() const;

	std::size_t total_objects() const { return objects.size(); }

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
