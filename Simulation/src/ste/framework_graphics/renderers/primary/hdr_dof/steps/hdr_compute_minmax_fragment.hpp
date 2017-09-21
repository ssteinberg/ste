// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_graphics.hpp>
#include <combined_image_sampler.hpp>
#include <array.hpp>

#include <hdr_dof_bokeh_parameters.hpp>

#include <task.hpp>
#include <cmd_draw.hpp>

namespace ste {
namespace graphics {

class hdr_compute_minmax_fragment : public gl::fragment_graphics<hdr_compute_minmax_fragment> {
	using Base = gl::fragment_graphics<hdr_compute_minmax_fragment>;

	gl::task<gl::cmd_draw> draw_task;

public:
	hdr_compute_minmax_fragment(const gl::rendering_system &rs)
		: Base(rs,
			   gl::device_pipeline_graphics_configurations{},
			   "fullscreen_triangle.vert", "hdr_compute_minmax.frag")
	{
		draw_task.attach_pipeline(pipeline());
	}
	~hdr_compute_minmax_fragment() noexcept {}

	hdr_compute_minmax_fragment(hdr_compute_minmax_fragment&&) = default;

	static lib::string name() { return "hdr_compute_minmax"; }

	static void setup_graphics_pipeline(const gl::rendering_system &rs,
										gl::pipeline_auditor_graphics &auditor) {
		gl::framebuffer_layout fb_layout;
		fb_layout[0] = gl::ignore_store(gl::format::r32_sfloat,
										gl::image_layout::shader_read_only_optimal);
		auditor.set_framebuffer_layout(fb_layout);
	}

	void bind_buffers(const gl::array<hdr_bokeh_parameters> &hdr_bokeh_parameters_buffer) {
		pipeline()["hdr_bokeh_parameters_buffer"] = gl::bind(hdr_bokeh_parameters_buffer);
	}
	void set_source(const gl::pipeline::combined_image_sampler &src) {
		pipeline()["hdr"] = gl::bind(src);
	}

	void attach_framebuffer(gl::framebuffer &fb) {
		pipeline().attach_framebuffer(fb);
	}
	const auto& get_framebuffer_layout() const {
		return pipeline().get_framebuffer_layout();
	}

	void record(gl::command_recorder &recorder) override final {
		recorder << draw_task(3, 1);
	}
};

}
}
