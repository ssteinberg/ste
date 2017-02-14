// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <buffer_usage.hpp>

#include <mesh_descriptor.hpp>

#include <element_buffer_object.hpp>
#include <vertex_buffer_object.hpp>
#include <vertex_array_object.hpp>
#include <shader_storage_buffer.hpp>

#include <gstack.hpp>

#include <memory>

namespace StE {
namespace Graphics {

class object_group_draw_buffers {
private:
	Core::vertex_array_object vao;

	mutable Core::gstack<mesh_descriptor> mesh_data_bo;
	mutable Core::gstack<mesh_draw_params> mesh_draw_params_bo;
	Core::gstack<object_vertex_data> vbo;
	Core::gstack<std::uint32_t> indices;

public:
	using vbo_type = Core::vertex_buffer_object<object_vertex_data, object_vertex_data::descriptor, decltype(vbo)::usage>;
	using elements_type = Core::element_buffer_object<std::uint32_t, decltype(indices)::usage>;
	using mesh_data_buffer_type = Core::shader_storage_buffer<mesh_descriptor, decltype(mesh_data_bo)::usage>;

public:
	object_group_draw_buffers() {
		auto vbo_buffer = Core::buffer_object_cast<vbo_type>(vbo.get_buffer());
		vao[0] = vbo_buffer[0];
		vao[1] = vbo_buffer[1];
		vao[2] = vbo_buffer[2];
	}

	auto& get_vao() const { return vao; }

	auto& get_mesh_data_stack() const { return mesh_data_bo; }
	auto& get_mesh_draw_params_stack() const { return mesh_draw_params_bo; }
	auto& get_vbo_stack() { return vbo; }
	auto& get_indices_stack() { return indices; }

	auto  get_vbo_buffer() const { return Core::buffer_object_cast<vbo_type>(vbo.get_buffer()); }
	auto  get_elements_buffer() const { return Core::buffer_object_cast<elements_type>(indices.get_buffer()); }
	auto& get_mesh_data_buffer() const { return mesh_data_bo.get_buffer(); }
	auto& get_mesh_draw_params_buffer() const { return mesh_draw_params_bo.get_buffer(); }

	auto size() const { return mesh_data_bo.size(); }

	void bind_buffers(int idx) const {
		get_vao().bind();
		get_elements_buffer().bind();
		get_mesh_data_buffer().bind_range(Core::shader_storage_layout_binding(idx), 0, size());
	}
};

}
}
