// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_compute.hpp>

#include <cmd_dispatch.hpp>

#include <voxel_storage.hpp>

namespace ste {
namespace graphics {

class voxelizer_subdivide_fragment : public gl::fragment_compute<voxelizer_subdivide_fragment> {
	using Base = gl::fragment_compute<voxelizer_subdivide_fragment>;

private:
	gl::task<gl::cmd_dispatch> dispatch_task;

	voxel_storage *voxels;

public:
	voxelizer_subdivide_fragment(const gl::rendering_system &rs,
								 voxel_storage *voxels)
		: Base(rs,
			   "sparse_voxelizer_subdivide.comp"),
		  voxels(voxels) {
		dispatch_task.attach_pipeline(pipeline());

		// Attach voxel storage resources
		pipeline()["voxels"] = gl::bind(gl::pipeline::storage_image(voxels->voxels_buffer_image()));
		pipeline()["voxels_counter_binding"] = gl::bind(voxels->voxels_counter_buffer());

		pipeline()["voxels_assembly_list_counter_binding"] = gl::bind(voxels->voxel_assembly_list_counter_buffer());
		pipeline()["voxels_assembly_list_binding"] = gl::bind(voxels->voxel_assembly_list_buffer());
		pipeline()["brick_assembly_counter_binding"] = gl::bind(voxels->brick_assembly_counter_buffer());
		pipeline()["brick_assembly_list_binding"] = gl::bind(voxels->brick_assembly_list_buffer());

		// Configure voxelization pipeline
		voxels->configure_voxel_pipeline(pipeline());
	}

	~voxelizer_subdivide_fragment() noexcept {}

	voxelizer_subdivide_fragment(voxelizer_subdivide_fragment &&) = default;

	static lib::string name() { return "voxelizer_subdivide"; }

	void record(gl::command_recorder &recorder) override final {
		recorder << dispatch_task(1, 1, 1);
	}
};

}
}
