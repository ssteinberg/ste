// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_compute.hpp>

#include <cmd_dispatch.hpp>

#include <voxel_storage.hpp>

namespace ste {
namespace graphics {

class voxelizer_embed_bricks_fragment : public gl::fragment_compute<voxelizer_embed_bricks_fragment> {
	using Base = gl::fragment_compute<voxelizer_embed_bricks_fragment>;

private:
	gl::task<gl::cmd_dispatch> dispatch_task;

	voxel_storage *voxels;

public:
	voxelizer_embed_bricks_fragment(const gl::rendering_system &rs,
									voxel_storage *voxels)
		: Base(rs,
			   "sparse_voxelizer_embed_bricks.comp"),
		  voxels(voxels) {
		dispatch_task.attach_pipeline(pipeline());

		// Attach voxel storage resources
		pipeline()["voxels"] = gl::bind(gl::pipeline::storage_image(voxels->voxels_buffer_image()));

		pipeline()["bricks_albedo"] = gl::bind(gl::pipeline::storage_image(voxels->albedo_bricks_image()));
		pipeline()["bricks_roughness"] = gl::bind(gl::pipeline::storage_image(voxels->roughness_bricks_image()));
		pipeline()["bricks_metadata"] = gl::bind(gl::pipeline::storage_image(voxels->metadata_bricks_image()));

		pipeline()["bricks_albedo_rgba8"] = gl::bind(gl::pipeline::combined_image_sampler(voxels->albedo_bricks_image_rgba8(),
																						  rs.get_creating_context().device().common_samplers_collection().linear_clamp_sampler()));
		pipeline()["bricks_roughness_rg8"] = gl::bind(gl::pipeline::combined_image_sampler(voxels->roughness_bricks_image_r8g8(),
																						   rs.get_creating_context().device().common_samplers_collection().linear_clamp_sampler()));
		pipeline()["bricks_metadata_rgba8"] = gl::bind(gl::pipeline::combined_image_sampler(voxels->metadata_bricks_image_rgba8(),
																							rs.get_creating_context().device().common_samplers_collection().linear_clamp_sampler()));

		pipeline()["bricks_counter_binding"] = gl::bind(voxels->bricks_counter_buffer());

		pipeline()["brick_assembly_counter_binding"] = gl::bind(voxels->brick_assembly_counter_buffer());
		pipeline()["brick_assembly_list_binding"] = gl::bind(voxels->brick_assembly_list_buffer());

		// Configure voxelization pipeline
		voxels->configure_voxel_pipeline(pipeline());
	}

	~voxelizer_embed_bricks_fragment() noexcept {}

	voxelizer_embed_bricks_fragment(voxelizer_embed_bricks_fragment &&) = default;

	static lib::string name() { return "voxelizer_embed_bricks"; }

	void record(gl::command_recorder &recorder) override final {
		recorder << dispatch_task(1, 1, 1);
	}
};

}
}
