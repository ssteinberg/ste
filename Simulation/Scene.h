// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "Object.h"
#include "ObjectVertexData.h"
#include "Material.h"

#include "Sampler.h"
#include "texture_handle.h"

#include "ElementBufferObject.h"
#include "VertexBufferObject.h"
#include "VertexArrayObject.h"
#include "IndirectDrawBufferObject.h"
#include "ShaderStorageBuffer.h"

#include "range.h"

#include <memory>
#include <vector>

namespace StE {
namespace Graphics {

class Scene {
private:
	struct material_texture_descriptor {
		LLR::texture_handle tex_handler;
	};
	struct material_descriptor {
		material_texture_descriptor diffuse;
		material_texture_descriptor specular;
		material_texture_descriptor heightmap;
		material_texture_descriptor alphamap;
	};
 
 	static constexpr auto buffer_usage = static_cast<LLR::BufferUsage::buffer_usage>(LLR::BufferUsage::BufferUsageDynamic | LLR::BufferUsage::BufferUsageSparse);
 	using vbo_type = LLR::VertexBufferObject<ObjectVertexData, ObjectVertexData::descriptor, buffer_usage>;
 	using elements_type = LLR::ElementBufferObject<unsigned, buffer_usage>;
 	using indirect_draw_buffer_type = LLR::IndirectDrawBuffer<LLR::IndirectMultiDrawElementsCommand, buffer_usage>;
 	using material_data_buffer_type = LLR::ShaderStorageBuffer<material_descriptor, buffer_usage>;

	LLR::SamplerMipmapped linear_sampler;
 
 	std::unique_ptr<LLR::VertexArrayObject> vao;
 	std::unique_ptr<vbo_type> vbo;
 	std::unique_ptr<elements_type> indices;
 	std::unique_ptr<indirect_draw_buffer_type> idb;
 	std::unique_ptr<material_data_buffer_type> matbo;
 
	std::vector<std::unique_ptr<Object>> objects;

	int total_vertices{ 0 };
	int total_indices{ 0 };

public:
	Scene();

	Object* create_object(const std::vector<ObjectVertexData> &vertices, const std::vector<unsigned> &indices, Material &&mat);

	void render() const;
};

}
}
