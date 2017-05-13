// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_graphics.hpp>
#include <task.hpp>
#include <cmd_draw.hpp>

namespace ste {
namespace graphics {

class hdr_bokeh_blur_fragment : public gl::fragment_graphics<hdr_bokeh_blur_fragment> {
	using Base = gl::fragment_graphics<hdr_bokeh_blur_fragment>;

	gl::task<gl::cmd_draw> draw_task;

public:
	hdr_bokeh_blur_fragment(const gl::rendering_system &rs)
		: Base(rs,
			   gl::device_pipeline_graphics_configurations{},
			   "passthrough.vert", "bokeh_bilateral_blur.frag")
	{
		draw_task.attach_pipeline(pipeline);
	}
	~hdr_bokeh_blur_fragment() noexcept {}

	static const std::string& name() { return "hdr_bokeh_blur"; }

	void attach_framebuffer(gl::framebuffer &fb) {
		pipeline.attach_framebuffer(fb);
	}

	void record(gl::command_recorder &recorder) override final {
		recorder << draw_task(3, 1);
	}
};

}
}
