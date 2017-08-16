// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <cmd_draw_indexed_indirect.hpp>
#include <deferred_gbuffer.hpp>

#include <fragment_graphics.hpp>
#include <scene.hpp>
#include <object_vertex_data.hpp>

namespace ste {
namespace graphics {

class scene_write_gbuffer_dispatch : public gl::fragment_graphics<scene_write_gbuffer_dispatch> {
	using Base = gl::fragment_graphics<scene_write_gbuffer_dispatch>;

private:
	gl::task<gl::cmd_draw_indexed_indirect> draw_task;

	const scene *s;
	const deferred_gbuffer *gbuffer;

public:
	scene_write_gbuffer_dispatch(const gl::rendering_system &rs,
								 const scene *s,
								 const deferred_gbuffer *gbuffer)
		: Base(rs,
			   gl::device_pipeline_graphics_configurations{},
			   "scene_transform.vert", "object.frag"),
		s(s),
		gbuffer(gbuffer)
	{
		draw_task.attach_pipeline(pipeline);
		draw_task.attach_vertex_buffer(s->get_object_group().get_draw_buffers().get_vertex_buffer());
		draw_task.attach_index_buffer(s->get_object_group().get_draw_buffers().get_index_buffer());
		draw_task.attach_indirect_buffer(s->get_idb());
	}
	~scene_write_gbuffer_dispatch() noexcept {}

protected:
	static const lib::string& name() { return "scene"; }

	static void setup_graphics_pipeline(const gl::rendering_system &rs,
										gl::pipeline_auditor_graphics &auditor) {
		gl::framebuffer_layout fb_layout;
		fb_layout[gl::pipeline_depth_attachment_location] = gl::load_store(gl::format::d32_sfloat,
																		   gl::image_layout::depth_stencil_attachment_optimal,
																		   gl::image_layout::depth_stencil_attachment_optimal);
		fb_layout[0] = gl::ignore_store(gl::format::r32g32b32a32_sfloat,
										gl::image_layout::color_attachment_optimal);
		fb_layout[1] = gl::ignore_store(gl::format::r32g32b32a32_sfloat,
										gl::image_layout::color_attachment_optimal);
		auditor.set_framebuffer_layout(fb_layout);

		gl::device_pipeline_graphics_configurations config;
		config.depth_op = gl::depth_operation(gl::compare_op::equal, 
											  false);
		config.rasterizer_op = gl::rasterizer_operation(gl::cull_mode::front_bit,
														gl::front_face::ccw);
		auditor.set_pipeline_settings(std::move(config));
		auditor.set_vertex_attributes(0, gl::vertex_attributes<object_vertex_data>());
	}

	void attach_framebuffer(gl::framebuffer &fb) {
		pipeline.attach_framebuffer(fb);
	}
	const auto& get_framebuffer_layout() const {
		return pipeline.get_framebuffer_layout();
	}

	void record(gl::command_recorder &recorder) override final {
		recorder << draw_task(s->get_object_group().get_draw_buffers().draw_count(), 
							  sizeof(gl::draw_indexed_indirect_command_std140));
	}
};

}
}
