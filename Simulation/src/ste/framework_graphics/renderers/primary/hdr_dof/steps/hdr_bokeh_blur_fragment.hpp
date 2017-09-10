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

class hdr_bokeh_blur_fragment : public gl::fragment_graphics<hdr_bokeh_blur_fragment> {
	using Base = gl::fragment_graphics<hdr_bokeh_blur_fragment>;

	gl::task<gl::cmd_draw> draw_task;

public:
	hdr_bokeh_blur_fragment(const gl::rendering_system &rs,
							gl::framebuffer_layout &&fb_layout)
		: Base(rs,
			   gl::device_pipeline_graphics_configurations{},
			   std::move(fb_layout),
			   "fullscreen_triangle.vert", "bokeh_bilateral_blur.frag")
	{
		draw_task.attach_pipeline(pipeline());
	}
	~hdr_bokeh_blur_fragment() noexcept {}

	hdr_bokeh_blur_fragment(hdr_bokeh_blur_fragment&&) = default;

	static lib::string name() { return "hdr_bokeh_blur"; }

	void bind_buffers(const gl::array<hdr_bokeh_parameters> &hdr_bokeh_parameters_buffer) {
		pipeline()["hdr_bokeh_parameters_buffer"] = gl::bind(hdr_bokeh_parameters_buffer);
	}
	void set_source(const gl::pipeline::combined_image_sampler &src) {
		pipeline()["hdr"] = gl::bind(src);
	}
	void set_aperture_parameters(float diameter, float focal_length) {
		pipeline()["aperture_t.aperture_focal_length"] = focal_length;
		pipeline()["aperture_t.aperture_diameter"] = diameter;
	}

	void attach_framebuffer(gl::framebuffer &fb) {
		pipeline().attach_framebuffer(fb);
	}

	void record(gl::command_recorder &recorder) override final {
		recorder << draw_task(3, 1);
	}
};

}
}
