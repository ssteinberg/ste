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

#include "ElementBufferObject.hpp"
#include "VertexBufferObject.hpp"
#include "VertexArrayObject.hpp"
#include "IndirectDrawBufferObject.hpp"
#include "ShaderStorageBuffer.hpp"

#include "ObjectVertexData.hpp"
#include "Material.hpp"
#include "SceneProperties.hpp"

#include "range.hpp"

#include "gstack.hpp"

#include <unordered_map>
#include <memory>

namespace StE {
namespace Graphics {

class ObjectGroup : public gpu_dispatchable, public entity_affine {
	using Base = gpu_dispatchable;

private:
	struct mesh_descriptor {
		glm::mat4 model, transpose_inverse_model;
		std::int32_t mat_idx;
		std::int32_t _unused[3];
	};

	using signal_connection_type = Object::signal_type::connection_type;

	struct object_information {
		std::size_t index;
		std::shared_ptr<signal_connection_type> connection;
	};

	using objects_map_type = std::unordered_map<std::shared_ptr<Object>, object_information>;

	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;

private:
 	Core::VertexArrayObject vao;

	mutable Core::gstack<mesh_descriptor> mesh_data_bo;
 	Core::gstack<ObjectVertexData> vbo;
	Core::gstack<std::uint32_t> indices;
	Core::gstack<Core::IndirectMultiDrawElementsCommand> idb;

private:
 	using vbo_type = Core::VertexBufferObject<ObjectVertexData, ObjectVertexData::descriptor, decltype(vbo)::usage>;
 	using elements_type = Core::ElementBufferObject<std::uint32_t, decltype(indices)::usage>;
 	using indirect_draw_buffer_type = Core::IndirectDrawBuffer<Core::IndirectMultiDrawElementsCommand, decltype(idb)::usage>;
	using mesh_data_buffer_type = Core::ShaderStorageBuffer<mesh_descriptor, decltype(mesh_data_bo)::usage>;

private:
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

	void draw_to_gbuffer(const deferred_gbuffer *gbuffer) { this->gbuffer = gbuffer; }

	void add_object(const std::shared_ptr<Object> &);
	void remove_all();

	void set_model_matrix(const glm::mat4 &m) override {
		entity_affine::set_model_matrix(m);

		object_program->set_uniform("view_matrix", m);
		object_program->set_uniform("trans_inverse_view_matrix", glm::transpose(glm::inverse(m)));
	}

	void bind_buffers() const;

protected:
	void update_dirty_buffers() const;

	void set_context_state() const override final;

public:
	void dispatch() const override final;
};

}
}
