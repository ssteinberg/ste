// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_compute.hpp>
#include <combined_image_sampler.hpp>
#include <texture.hpp>
#include <array.hpp>

#include <hdr_dof_bokeh_parameters.hpp>

#include <task.hpp>
#include <cmd_dispatch.hpp>

namespace ste {
namespace graphics {

class hdr_compute_minmax_fragment : public gl::fragment_compute {
	using Base = gl::fragment_compute;

	gl::task<gl::cmd_dispatch> dispatch_task;
	glm::u32vec2 extent;

public:
	hdr_compute_minmax_fragment(const gl::rendering_system &rs)
		: Base(rs,
			   "hdr_compute_minmax.comp")
	{
		dispatch_task.attach_pipeline(pipeline);
	}
	~hdr_compute_minmax_fragment() noexcept {}

	static const std::string& name() { return "hdr_compute_minmax"; }

	void bind_buffers(const gl::array<hdr_bokeh_parameters> &hdr_bokeh_parameters_buffer) {
		pipeline["hdr_bokeh_parameters_buffer"] = gl::bind(hdr_bokeh_parameters_buffer);
	}
	void set_source(const gl::pipeline::combined_image_sampler &src) {
		pipeline["hdr"] = gl::bind(src);
	}
	void set_destination(const gl::pipeline::image &hdr_lums,
						 const glm::u32vec2 &extent) {
		pipeline["hdr_lums"] = gl::bind(hdr_lums);
		this->extent = extent;
	}

	void record(gl::command_recorder &recorder) override final {
		recorder << dispatch_task(extent.x / 32, extent.y / 32, 1);
	}
};

}
}
