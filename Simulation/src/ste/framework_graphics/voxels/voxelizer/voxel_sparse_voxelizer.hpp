// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment.hpp>

#include <voxelizer_generate_voxel_list_fragment.hpp>
#include <voxelizer_subdivide_fragment.hpp>

#include <scene.hpp>

#include <voxel_storage.hpp>

namespace ste {
namespace graphics {

class voxel_sparse_voxelizer : public gl::fragment {
	using Base = gl::fragment;

private:
	voxelizer_generate_voxel_list_fragment generate_voxel_list;
	voxelizer_subdivide_fragment subdivide;

	const voxel_storage *voxels;

public:
	voxel_sparse_voxelizer(const gl::rendering_system &rs,
						   voxel_storage *voxels,
						   const scene *s)
		: Base(rs.get_creating_context()),
		generate_voxel_list(rs,
							voxels,
							s),
		subdivide(rs,
				  voxels),
		voxels(voxels)
	{}
	~voxel_sparse_voxelizer() noexcept {}

	voxel_sparse_voxelizer(voxel_sparse_voxelizer &&) = default;

	void record(gl::command_recorder &recorder) override final {
		// Create voxel list
		recorder << generate_voxel_list;

		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::fragment_shader,
																  gl::pipeline_stage::compute_shader,
																  gl::buffer_memory_barrier(voxels->voxel_list_buffer(),
																							gl::access_flags::shader_write,
																							gl::access_flags::shader_read),
																  gl::buffer_memory_barrier(voxels->voxel_list_counter_buffer(),
																							gl::access_flags::shader_write,
																							gl::access_flags::shader_read)));

		// Subdivide tree
		recorder << subdivide;
	}
};

}
}
