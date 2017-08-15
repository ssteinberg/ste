// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <cmd_draw_indexed_indirect.hpp>

#include <fragment_graphics.hpp>
#include <scene.hpp>

namespace ste {
namespace graphics {

template <bool front_face>
class scene_prepopulate_depth_dispatch : public gl::fragment_graphics<scene_prepopulate_depth_dispatch<front_face>> {
	using Base = gl::fragment_graphics<scene_prepopulate_depth_dispatch<front_face>>;

private:
	gl::task<gl::cmd_draw_indexed_indirect> draw_task;

	const scene *s;

public:
	scene_prepopulate_depth_dispatch(const gl::rendering_system &rs,
									 const scene *s)
		: Base(rs,
			   gl::device_pipeline_graphics_configurations{},
			   "scene_transform.vert", "scene_prepopulate_depth.frag"),
		s(s)
	{
		draw_task.attach_pipeline(pipeline);
		draw_task.attach_vertex_buffer(s->);
		draw_task.attach_index_buffer(s->);
		draw_task.attach_indirect_buffer(s->);
	}
	~scene_prepopulate_depth_dispatch() noexcept {}

protected:
	static const lib::string& name() { return "prepopulate_depth"; }

	static void setup_graphics_pipeline(const gl::rendering_system &rs,
										gl::pipeline_auditor_graphics &auditor) {
		gl::framebuffer_layout fb_layout;
		if (front_face)
			fb_layout[gl::pipeline_depth_attachment_location] = gl::clear_store(gl::format::d32_sfloat,
																				gl::image_layout::depth_stencil_attachment_optimal);
		else
			fb_layout[gl::pipeline_depth_attachment_location] = gl::ignore_store(gl::format::d32_sfloat,
																				 gl::image_layout::depth_stencil_attachment_optimal);
		auditor.set_framebuffer_layout(fb_layout);

		gl::device_pipeline_graphics_configurations config;
		config.depth_op = gl::depth_operation(gl::compare_op::greater);
		config.rasterizer_op = gl::rasterizer_operation(front_face ?
														gl::cull_mode::back_bit :
														gl::cull_mode::front_bit,
														gl::front_face::ccw);
		config.blend_constants
		auditor.set_pipeline_settings(std::move(config));
	}

	void attach_framebuffer(gl::framebuffer &fb) {
		pipeline.attach_framebuffer(fb);
	}
	const auto& get_framebuffer_layout() const {
		return pipeline.get_framebuffer_layout();
	}

	void record(gl::command_recorder &recorder) override final {
		recorder << draw_task(s->draw_count, sizeof(gl::draw_indexed_indirect_command_std140));
	}
};

}
}
