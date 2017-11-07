// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_graphics.hpp>

#include <cmd_draw_indexed_indirect.hpp>

#include <scene.hpp>
#include <object_vertex_data.hpp>

#include <voxel_storage.hpp>

namespace ste {
namespace graphics {

class voxelizer_generate_voxel_list_fragment : public gl::fragment_graphics<voxelizer_generate_voxel_list_fragment> {
	using Base = gl::fragment_graphics<voxelizer_generate_voxel_list_fragment>;

private:
	static constexpr std::uint32_t voxelizer_fb_extent = 16384;

private:
	gl::task<gl::cmd_draw_indexed_indirect> draw_task;

	const scene *s;
	voxel_storage *voxels;

	lib::unique_ptr<gl::framebuffer> empty_fb;

	std::uint32_t offset, count;

public:
	voxelizer_generate_voxel_list_fragment(const gl::rendering_system &rs,
										   voxel_storage *voxels,
										   const scene *s)
		: Base(rs,
			   gl::device_pipeline_graphics_configurations{},
			   "sparse_voxelizer.vert",
			   "sparse_voxelizer.geom",
			   "sparse_voxelizer.frag"),
		  s(s),
		  voxels(voxels),
		  empty_fb(lib::allocate_unique<gl::framebuffer>(rs.get_creating_context(),
														 "voxelizer framebuffer",
														 gl::framebuffer_layout(),
														 glm::u32vec2{ voxelizer_fb_extent, voxelizer_fb_extent })) {
		draw_task.attach_pipeline(pipeline());
		draw_task.attach_vertex_buffer(s->get_object_group().get_draw_buffers().get_vertex_buffer());
		draw_task.attach_index_buffer(s->get_object_group().get_draw_buffers().get_index_buffer());

		// Attach empty framebuffer
		pipeline().attach_framebuffer(*empty_fb);

		// Configure voxelization pipeline
		pipeline()["voxels_assembly_list_counter_binding"] = gl::bind(voxels->voxel_assembly_list_counter_buffer());
		pipeline()["voxels_assembly_list_binding"] = gl::bind(voxels->voxel_assembly_list_buffer());
		voxels->configure_voxel_pipeline(pipeline());
	}

	~voxelizer_generate_voxel_list_fragment() noexcept {}

	voxelizer_generate_voxel_list_fragment(voxelizer_generate_voxel_list_fragment &&) = default;

	static lib::string name() { return "voxelizer_generate_voxel_list"; }

	static void setup_graphics_pipeline(const gl::rendering_system &rs,
										gl::pipeline_auditor_graphics &auditor) {
		auditor.set_framebuffer_layout(gl::framebuffer_layout());

		gl::device_pipeline_graphics_configurations config;
		config.rasterizer_op = gl::rasterizer_operation(gl::cull_mode::none,
														gl::front_face::cw);
		auditor.set_pipeline_settings(std::move(config));
		auditor.set_vertex_attributes(0, gl::vertex_attributes<object_vertex_data>());
	}

	void prepare() {
		offset = 0;
		count = 0;
	}
	bool next(std::uint32_t max_vertices_count) {
		const auto &sizes = s->get_object_group().object_size_list();

		offset += count;
		count = 0;
		for (std::uint32_t vc = 0; vc < max_vertices_count && offset + count < sizes.size(); ++count)
			vc += sizes[offset + count];

		draw_task.attach_indirect_buffer(s->get_idb().get(), offset);

		return offset + count == sizes.size();
	}

	void record(gl::command_recorder &recorder) override final {
		// Voxelize
		recorder << draw_task(count);
	}
};

}
}
