// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <buffer_usage.hpp>
#include <stable_vector.hpp>
#include <mesh_descriptor.hpp>

namespace ste {
namespace graphics {

class object_group_draw_buffers {
private:
	mutable gl::stable_vector<mesh_descriptor> mesh_data_bo;
	mutable gl::stable_vector<mesh_draw_params> mesh_draw_params_bo;
	gl::stable_vector<object_vertex_data> vbo;
	gl::stable_vector<std::uint32_t> indices;

public:
	object_group_draw_buffers(const ste_context &ctx)
		: mesh_data_bo(ctx, 
					   gl::buffer_usage::storage_buffer,
					   "mesh_data buffer"),
		  mesh_draw_params_bo(ctx, 
							  gl::buffer_usage::storage_buffer,
							  "mesh_draw_params buffer"),
		  vbo(ctx, 
			  gl::buffer_usage::vertex_buffer,
			  "vertex attributes buffer"),
		  indices(ctx, 
				  gl::buffer_usage::index_buffer,
				  "indices buffer") {}

	auto &get_mesh_data_buffer() { return mesh_data_bo; }
	auto &get_mesh_data_buffer() const { return mesh_data_bo; }
	auto &get_mesh_draw_params_buffer() { return mesh_draw_params_bo; }
	auto &get_mesh_draw_params_buffer() const { return mesh_draw_params_bo; }

	auto &get_vertex_buffer() { return vbo; }
	auto &get_vertex_buffer() const { return vbo; }
	auto &get_index_buffer() { return indices; }
	auto &get_index_buffer() const { return indices; }
};

}
}
