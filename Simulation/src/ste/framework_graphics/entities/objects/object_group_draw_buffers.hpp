// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "buffer_usage.hpp"

#include "mesh_descriptor.hpp"

#include "ElementBufferObject.hpp"
#include "VertexBufferObject.hpp"
#include "VertexArrayObject.hpp"
#include "IndirectDrawBufferObject.hpp"
#include "ShaderStorageBuffer.hpp"
#include "AtomicCounterBufferObject.hpp"

#include "gstack.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class object_group_draw_buffers {
private:
 	Core::VertexArrayObject vao;

	mutable Core::gstack<mesh_descriptor> mesh_data_bo;
 	Core::gstack<ObjectVertexData> vbo;
	Core::gstack<std::uint32_t> indices;
	mutable Core::gstack<Core::IndirectMultiDrawElementsCommand, true> culled_indirect_command;
	Core::gstack<std::uint32_t> id_to_drawid;
	mutable Core::AtomicCounterBufferObject<> culled_objects_counter;

public:
 	using vbo_type = Core::VertexBufferObject<ObjectVertexData, ObjectVertexData::descriptor, decltype(vbo)::usage>;
 	using elements_type = Core::ElementBufferObject<std::uint32_t, decltype(indices)::usage>;
 	using indirect_draw_buffer_type = Core::IndirectDrawBuffer<Core::IndirectMultiDrawElementsCommand, decltype(culled_indirect_command)::usage>;
	using mesh_data_buffer_type = Core::ShaderStorageBuffer<mesh_descriptor, decltype(mesh_data_bo)::usage>;

public:
	object_group_draw_buffers() : culled_objects_counter(1) {
		auto vbo_buffer = Core::buffer_object_cast<vbo_type>(vbo.get_buffer());
		vao[0] = vbo_buffer[0];
		vao[1] = vbo_buffer[1];
		vao[2] = vbo_buffer[2];
		vao[3] = vbo_buffer[3];
	}

	void clear_indirect_command_buffer() const {
		std::uint32_t zero = 0;
		culled_indirect_command.overwrite_all(gli::format::FORMAT_R32_UINT_PACK32, &zero, 0, culled_indirect_command.size());
		culled_objects_counter.clear(gli::format::FORMAT_R32_UINT_PACK32, &zero);
	}

	auto& get_vao() const { return vao; }

	auto& get_mesh_data_stack() { return mesh_data_bo; }
	auto& get_mesh_data_stack() const { return mesh_data_bo; }
	auto& get_vbo_stack() { return vbo; }
	auto& get_indices_stack() { return indices; }
	auto& get_id_to_drawid_stack() { return id_to_drawid; }
	auto& get_culled_indirect_command_stack() { return culled_indirect_command; }

	auto  get_vbo_buffer() const { return Core::buffer_object_cast<vbo_type>(vbo.get_buffer()); }
	auto  get_elements_buffer() const { return Core::buffer_object_cast<elements_type>(indices.get_buffer()); }
	auto  get_culled_indirect_draw() const { return Core::buffer_object_cast<indirect_draw_buffer_type>(culled_indirect_command.get_buffer()); }
	auto& get_culled_objects_counter() const { return culled_objects_counter; }

	auto& get_mesh_data_buffer() const { return mesh_data_bo.get_buffer(); }
	auto& get_id_to_drawid_buffer() const { return id_to_drawid.get_buffer(); }
	auto& get_culled_indirect_command_buffer() const { return culled_indirect_command.get_buffer(); }

	auto size() const { return culled_indirect_command.size(); }
};

}
}
