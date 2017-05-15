// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_compute.hpp>
#include <texture.hpp>
#include <array.hpp>

#include <hdr_dof_bokeh_parameters.hpp>

#include <task.hpp>
#include <cmd_dispatch.hpp>
#include <cmd_fill_buffer.hpp>

namespace ste {
namespace graphics {

class hdr_compute_histogram_fragment : public gl::fragment_compute {
	using Base = gl::fragment_compute;

	gl::task<gl::cmd_dispatch> dispatch_task;
	glm::u32vec2 extent;

public:
	hdr_compute_histogram_fragment(const gl::rendering_system &rs)
		: Base(rs,
			   "hdr_create_histogram.comp")
	{
		dispatch_task.attach_pipeline(pipeline);
	}
	~hdr_compute_histogram_fragment() noexcept {}

	static const std::string& name() { return "hdr_compute_histogram"; }

	void bind_buffers(const gl::array<gl::std430<std::uint32_t>> &histogram_data, 
					  const gl::array<hdr_bokeh_parameters> &hdr_bokeh_parameters_buffer) {
		pipeline["histogram_data"] = gl::bind(histogram_data);
		pipeline["hdr_bokeh_parameters_buffer"] = gl::bind(hdr_bokeh_parameters_buffer);
	}
	void set_source(const gl::texture_generic &hdr_lums,
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
