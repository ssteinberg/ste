// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_compute.hpp>

#include <cmd_dispatch.hpp>

#include <voxel_storage.hpp>

namespace ste {
namespace graphics {

class voxelizer_clear_root : public gl::fragment_compute<voxelizer_clear_root> {
	using Base = gl::fragment_compute<voxelizer_clear_root>;

private:
	gl::task<gl::cmd_dispatch> dispatch_task;

	voxel_storage *voxels;

public:
	voxelizer_clear_root(const gl::rendering_system &rs,
								 voxel_storage *voxels)
		: Base(rs,
			   "sparse_voxelizer_clear_root.comp"),
		voxels(voxels) {
		dispatch_task.attach_pipeline(pipeline());

		// Configure voxelization pipeline
		pipeline()["voxels"] = gl::bind(gl::pipeline::storage_image(voxels->voxels_buffer_image()));
		voxels->configure_voxel_pipeline(pipeline());
	}

	~voxelizer_clear_root() noexcept {}

	voxelizer_clear_root(voxelizer_clear_root &&) = default;

	static lib::string name() { return "voxelizer_clear_root"; }

	void record(gl::command_recorder &recorder) override final {
		recorder << dispatch_task(1, 1, 1);
	}
};

}
}
