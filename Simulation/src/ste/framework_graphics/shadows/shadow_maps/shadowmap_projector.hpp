// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_graphics.hpp>

#include <scene.hpp>

namespace ste {
namespace graphics {

class shadowmap_projector : public gl::fragment_graphics<shadowmap_projector> {
	using Base = gl::fragment_graphics<shadowmap_projector>;

private:
	gl::task<gl::cmd_draw_indexed_indirect> draw_task;
	const scene *s;

public:
	shadowmap_projector(const gl::rendering_system &rs,
						const scene *s)
		: Base(rs,
			   gl::device_pipeline_graphics_configurations{},
			   "shadow_cubemap.vert", "shadow_cubemap.geom"),
		s(s)
	{
		draw_task.attach_pipeline(pipeline);
		draw_task.attach_vertex_buffer(s->get_object_group().get_draw_buffers().get_vertex_buffer());
		draw_task.attach_index_buffer(s->get_object_group().get_draw_buffers().get_index_buffer());
		draw_task.attach_indirect_buffer(s->get_idb());
	}
	~shadowmap_projector() noexcept {}

	shadowmap_projector(shadowmap_projector&&) = default;

	static const lib::string& name() { return "shadow_project"; }

	static void setup_graphics_pipeline(const gl::rendering_system &rs,
										gl::pipeline_auditor_graphics &auditor) {
		gl::framebuffer_layout fb_layout;
		fb_layout[gl::pipeline_depth_attachment_location] = gl::clear_store(gl::format::d32_sfloat,
																			gl::image_layout::depth_stencil_attachment_optimal);
		auditor.set_framebuffer_layout(fb_layout);

		gl::device_pipeline_graphics_configurations config;
		config.depth_op = gl::depth_operation(gl::compare_op::greater);
		config.rasterizer_op = gl::rasterizer_operation(gl::cull_mode::back_bit,
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
		auto draw_count = s->get_object_group().get_draw_buffers().draw_count();
		auto stride = sizeof(gl::draw_indexed_indirect_command_std140);
		recorder << draw_task(static_cast<std::uint32_t>(draw_count),
							  static_cast<std::uint32_t>(stride));
	}
};

}
}
