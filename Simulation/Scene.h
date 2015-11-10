// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "StEngineControl.h"

#include "Object.h"
#include "ObjectVertexData.h"
#include "Material.h"

#include "renderable.h"

#include "Sampler.h"
#include "texture_handle.h"

#include "ElementBufferObject.h"
#include "VertexBufferObject.h"
#include "VertexArrayObject.h"
#include "IndirectDrawBufferObject.h"
#include "ShaderStorageBuffer.h"

#include "range.h"

#include <memory>
#include <unordered_map>

namespace StE {
namespace Graphics {

class Scene : public renderable {
private:
	struct material_texture_descriptor {
		LLR::texture_handle tex_handler;
	};
	struct material_descriptor {
		material_texture_descriptor diffuse;
		material_texture_descriptor specular;
		material_texture_descriptor normalmap;
		material_texture_descriptor alphamap;
		BRDF::brdf_descriptor brdf;
		float emission;
	};
	struct mesh_descriptor {
		glm::mat4 model, transpose_inverse_model;
	};

	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type;
 
private:
 	static constexpr auto buffer_usage = static_cast<LLR::BufferUsage::buffer_usage>(LLR::BufferUsage::BufferUsageDynamic | LLR::BufferUsage::BufferUsageSparse);
 	using vbo_type = LLR::VertexBufferObject<ObjectVertexData, ObjectVertexData::descriptor, buffer_usage>;
 	using elements_type = LLR::ElementBufferObject<unsigned, buffer_usage>;
 	using indirect_draw_buffer_type = LLR::IndirectDrawBuffer<LLR::IndirectMultiDrawElementsCommand, buffer_usage>;
 	using material_data_buffer_type = LLR::ShaderStorageBuffer<material_descriptor, buffer_usage>;

	static constexpr auto persistent_buffer_usage = static_cast<LLR::BufferUsage::buffer_usage>(LLR::BufferUsage::BufferUsageMapCoherent | LLR::BufferUsage::BufferUsageMapWrite | LLR::BufferUsage::BufferUsageMapPersistent);
	using mesh_data_buffer_type = LLR::ShaderStorageBuffer<mesh_descriptor, persistent_buffer_usage>;

	LLR::SamplerMipmapped linear_sampler;
 
 	std::unique_ptr<LLR::VertexArrayObject> vao;
 	std::unique_ptr<vbo_type> vbo;
 	std::unique_ptr<elements_type> indices;
 	std::unique_ptr<indirect_draw_buffer_type> idb;
	std::unique_ptr<material_data_buffer_type> matbo;
	std::unique_ptr<mesh_data_buffer_type> mesh_data_bo;

	LLR::mapped_buffer_object_unique_ptr<mesh_descriptor, persistent_buffer_usage> mesh_data_bo_ptr;
 
	std::unordered_map<int, std::shared_ptr<Object>> objects;
	int object_count{ 0 };

	int total_vertices{ 0 };
	int total_indices{ 0 };

	mutable std::vector<range<>> ranges_to_lock;

	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

private:
	void resize_mesh_data_bo();

public:
	Scene(const StEngineControl &ctx);

	void add_object(const std::shared_ptr<Object> &obj);

	void prepare() const override final;
	void render() const override final;
	void finalize() const override final;

	void set_model_matrix(const glm::mat4 &m) {
		get_program()->set_uniform("view_matrix", m);
		get_program()->set_uniform("trans_inverse_view_matrix", glm::transpose(glm::inverse(m)));
	}
};

}
}
