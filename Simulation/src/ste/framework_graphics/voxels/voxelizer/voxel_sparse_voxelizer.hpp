// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment.hpp>

#include <voxelizer_clear_root.hpp>
#include <voxelizer_generate_voxel_list_fragment.hpp>
#include <voxelizer_subdivide_fragment.hpp>

#include <scene.hpp>

#include <voxel_storage.hpp>

namespace ste {
namespace graphics {

class voxel_sparse_voxelizer : public gl::fragment {
	using Base = gl::fragment;

	static constexpr std::uint32_t max_vertices_per_voxelization_chunk = 20000;

private:
	voxelizer_clear_root clear_root;
	voxelizer_generate_voxel_list_fragment generate_voxel_list;
	voxelizer_subdivide_fragment subdivide;

	const voxel_storage *voxels;

public:
	voxel_sparse_voxelizer(const gl::rendering_system &rs,
						   voxel_storage *voxels,
						   const scene *s)
		: Base(rs.get_creating_context()),
		clear_root(rs, voxels),
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
		// Clear root and voxel counter
		recorder
			<< clear_root
			<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
															 gl::pipeline_stage::compute_shader,
															 gl::image_memory_barrier(voxels->voxels_buffer_image().get_image(),
																					  gl::image_layout::general,
																					  gl::image_layout::general,
																					  gl::access_flags::shader_write,
																					  gl::access_flags::shader_write)));
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::pipeline_stage::transfer,
																  gl::buffer_memory_barrier(voxels->voxels_counter_buffer(),
																							gl::access_flags::shader_read | gl::access_flags::shader_write,
																							gl::access_flags::transfer_write),
																  gl::buffer_memory_barrier(voxels->voxel_assembly_list_counter_buffer(),
																							gl::access_flags::shader_read | gl::access_flags::shader_write,
																							gl::access_flags::transfer_write)));
		voxels->clear(recorder);
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
																  gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::buffer_memory_barrier(voxels->voxels_counter_buffer(),
																							gl::access_flags::transfer_write,
																							gl::access_flags::shader_read | gl::access_flags::shader_write),
																  gl::buffer_memory_barrier(voxels->voxel_assembly_list_counter_buffer(),
																							gl::access_flags::transfer_write,
																							gl::access_flags::shader_read | gl::access_flags::shader_write)));

		// Create voxel tree in chunks
		generate_voxel_list.prepare();
		while (true) {
			// Prepare next chunk
			const bool done = generate_voxel_list.next(max_vertices_per_voxelization_chunk);

			recorder
				// Create voxel list
				<< generate_voxel_list
				<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::fragment_shader,
																 gl::pipeline_stage::compute_shader,
																 gl::buffer_memory_barrier(voxels->voxel_assembly_list_buffer(),
																						   gl::access_flags::shader_write,
																						   gl::access_flags::shader_read | gl::access_flags::shader_write),
																 gl::buffer_memory_barrier(voxels->voxel_assembly_list_counter_buffer(),
																						   gl::access_flags::shader_write,
																						   gl::access_flags::shader_read)))
				// Subdivide tree
				<< subdivide;

			if (done)
				break;

			recorder
				<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
																 gl::pipeline_stage::fragment_shader,
																 gl::buffer_memory_barrier(voxels->voxel_assembly_list_counter_buffer(),
																						   gl::access_flags::shader_read | gl::access_flags::shader_write,
																						   gl::access_flags::shader_read | gl::access_flags::shader_write),
																 gl::buffer_memory_barrier(voxels->voxel_assembly_list_buffer(),
																						   gl::access_flags::shader_read | gl::access_flags::shader_write,
																						   gl::access_flags::shader_read | gl::access_flags::shader_write)));
		};
	}
};

}
}
