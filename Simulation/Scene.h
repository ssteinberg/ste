// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "StEngineControl.h"

#include "Object.h"
#include "ObjectVertexData.h"
#include "Material.h"
#include "SceneProperties.h"

#include "renderable.h"

#include "Sampler.h"

#include "ElementBufferObject.h"
#include "VertexBufferObject.h"
#include "VertexArrayObject.h"
#include "IndirectDrawBufferObject.h"
#include "ShaderStorageBuffer.h"

#include "range.h"

#include "gstack.h"

#include <memory>
#include <unordered_map>

namespace StE {
namespace Graphics {

class Scene : public renderable {
private:
	struct mesh_descriptor {
		glm::mat4 model, transpose_inverse_model;
		int mat_idx;
	};

	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;
 
private:
 	LLR::VertexArrayObject vao;

	mutable LLR::gstack<mesh_descriptor> mesh_data_bo;
 	LLR::gstack<ObjectVertexData> vbo;
	LLR::gstack<unsigned> indices;
	LLR::gstack<LLR::IndirectMultiDrawElementsCommand> idb;

 	using vbo_type = LLR::VertexBufferObject<ObjectVertexData, ObjectVertexData::descriptor, decltype(vbo)::usage>;
 	using elements_type = LLR::ElementBufferObject<unsigned, decltype(indices)::usage>;
 	using indirect_draw_buffer_type = LLR::IndirectDrawBuffer<LLR::IndirectMultiDrawElementsCommand, decltype(idb)::usage>;
	using mesh_data_buffer_type = LLR::ShaderStorageBuffer<mesh_descriptor, decltype(mesh_data_bo)::usage>;

private:
	std::unordered_map<int, std::shared_ptr<Object>> objects;

	std::size_t total_vertices{ 0 };
	std::size_t total_indices{ 0 };

	mutable std::vector<range<>> ranges_to_lock;

	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

	SceneProperties *scene_props;

protected:
	void bind_buffers() const;

public:
	Scene(const StEngineControl &ctx, SceneProperties *props);

	void add_object(const std::shared_ptr<Object> &obj);

	void prepare() const override final;
	void render() const override final;
	void finalize() const override final;

	void set_model_matrix(const glm::mat4 &m) {
		get_program()->set_uniform("view_matrix", m);
		get_program()->set_uniform("trans_inverse_view_matrix", glm::transpose(glm::inverse(m)));
	}

	SceneProperties *scene_properties() { return scene_props; }
	const SceneProperties *scene_properties() const { return scene_props; }
};

}
}
