// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "entity.h"
#include "Object.h"
#include "renderable.h"

#include "ElementBufferObject.h"
#include "VertexBufferObject.h"
#include "VertexArrayObject.h"
#include "IndirectDrawBufferObject.h"
#include "ShaderStorageBuffer.h"

#include "ObjectVertexData.h"
#include "Material.h"
#include "SceneProperties.h"

#include "range.h"

#include "gstack.h"

#include <unordered_map>
#include <memory>

namespace StE {
namespace Graphics {

class ObjectGroup : public renderable, public entity {
private:
	struct mesh_descriptor {
		glm::mat4 model, transpose_inverse_model;
		std::int32_t mat_idx;
		std::int32_t _unused[3]; 
	};
 
private:
 	LLR::VertexArrayObject vao;

	mutable LLR::gstack<mesh_descriptor> mesh_data_bo;
 	LLR::gstack<ObjectVertexData> vbo;
	LLR::gstack<std::uint32_t> indices;
	LLR::gstack<LLR::IndirectMultiDrawElementsCommand> idb;

 	using vbo_type = LLR::VertexBufferObject<ObjectVertexData, ObjectVertexData::descriptor, decltype(vbo)::usage>;
 	using elements_type = LLR::ElementBufferObject<std::uint32_t, decltype(indices)::usage>;
 	using indirect_draw_buffer_type = LLR::IndirectDrawBuffer<LLR::IndirectMultiDrawElementsCommand, decltype(idb)::usage>;
	using mesh_data_buffer_type = LLR::ShaderStorageBuffer<mesh_descriptor, decltype(mesh_data_bo)::usage>;

private:
	SceneProperties *scene_props;
	std::unordered_map<int, std::shared_ptr<Object>> entities;

	std::size_t total_vertices{ 0 };
	std::size_t total_indices{ 0 };

	mutable std::vector<range<>> ranges_to_lock;

protected:
	void bind_buffers() const;
	void update_dirty_buffers() const;

public:
	ObjectGroup(SceneProperties *props);

	void add_entity(const std::shared_ptr<Object> &entity);
	
	void prepare() const;
	void render() const;
	void finalize() const;
};

}
}
