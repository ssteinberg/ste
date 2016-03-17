// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "entity.hpp"
#include "Object.hpp"
#include "gpu_task.hpp"

#include "gl_current_context.hpp"
#include "GLSLProgram.hpp"

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

class ObjectGroup : public gpu_task, public entity_affine {	
	using Base = gpu_task;
	
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
 	LLR::VertexArrayObject vao;

	mutable LLR::gstack<mesh_descriptor> mesh_data_bo;
 	LLR::gstack<ObjectVertexData> vbo;
	LLR::gstack<std::uint32_t> indices;
	LLR::gstack<LLR::IndirectMultiDrawElementsCommand> idb;
	
private:
 	using vbo_type = LLR::VertexBufferObject<ObjectVertexData, ObjectVertexData::descriptor, decltype(vbo)::usage>;
 	using elements_type = LLR::ElementBufferObject<std::uint32_t, decltype(indices)::usage>;
 	using indirect_draw_buffer_type = LLR::IndirectDrawBuffer<LLR::IndirectMultiDrawElementsCommand, decltype(idb)::usage>;
	using mesh_data_buffer_type = LLR::ShaderStorageBuffer<mesh_descriptor, decltype(mesh_data_bo)::usage>;

private:
	SceneProperties *scene_props;
	objects_map_type objects;

	std::size_t total_vertices{ 0 };
	std::size_t total_indices{ 0 };

	mutable std::vector<Object*> signalled_objects;
	mutable std::vector<range<>> ranges_to_lock;
	
	std::shared_ptr<LLR::GLSLProgram> object_program;
	
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

protected:
	void bind_buffers() const;
	void update_dirty_buffers() const;

public:
	ObjectGroup(StEngineControl &ctx, SceneProperties *props);
	~ObjectGroup();

	void add_object(const std::shared_ptr<Object> &);
	void remove_all();
	
	void set_model_matrix(const glm::mat4 &m) override {
		entity_affine::set_model_matrix(m);
		
		object_program->set_uniform("view_matrix", m); 
		object_program->set_uniform("trans_inverse_view_matrix", glm::transpose(glm::inverse(m)));
	}
	
protected:
	void set_context_state() const override final {
		Base::set_context_state();
		
		LLR::gl_current_context::get()->enable_depth_test();
		LLR::gl_current_context::get()->enable_state(LLR::context_state_name::CULL_FACE);
		
		bind_buffers();
		object_program->bind();
	}
	
	void dispatch() const override final {
		update_dirty_buffers();
		
		LLR::gl_current_context::get()->draw_multi_elements_indirect<elements_type::T>(GL_TRIANGLES, 0, idb.size(), 0);
		
		for (auto &r : ranges_to_lock)
			mesh_data_bo.lock_range(r);
		ranges_to_lock.clear();
		
		LLR::gl_current_context::get()->disable_state(LLR::context_state_name::CULL_FACE);
		LLR::gl_current_context::get()->disable_depth_test();
	}
};

}
}
