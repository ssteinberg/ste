// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "Object.h"
#include "ObjectVertexData.h"
#include "Material.h"

#include "texture_pool.h"

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
		std::uint64_t tex_handler{ 0 };
	};
	struct material_descriptor {
		material_texture_descriptor diffuse;
		material_texture_descriptor specular;
		material_texture_descriptor heightmap;
		material_texture_descriptor alphamap;
		material_texture_descriptor tex0;
		material_texture_descriptor tex1;
		material_texture_descriptor tex2;
		material_texture_descriptor tex3;
	};

	static constexpr auto buffer_usage = static_cast<LLR::BufferUsage::buffer_usage>(LLR::BufferUsage::BufferUsageDynamic | LLR::BufferUsage::BufferUsageSparse);
	using vbo_type = LLR::VertexBufferObject<ObjectVertexData, ObjectVertexData::descriptor, buffer_usage>;
	using elements_type = LLR::ElementBufferObject<unsigned, buffer_usage>;
	using indirect_draw_buffer_type = LLR::IndirectDrawBuffer<buffer_usage>;
	using material_data_buffer_type = LLR::ShaderStorageBuffer<material_descriptor, buffer_usage>;

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
