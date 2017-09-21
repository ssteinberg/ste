// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_compute.hpp>
#include <combined_image_sampler.hpp>
#include <array.hpp>

#include <hdr_dof_bokeh_parameters.hpp>

#include <task.hpp>
#include <cmd_dispatch.hpp>
#include <cmd_fill_buffer.hpp>

namespace ste {
namespace graphics {

class hdr_compute_histogram_fragment : public gl::fragment_compute<hdr_compute_histogram_fragment> {
	using Base = gl::fragment_compute<hdr_compute_histogram_fragment>;

	gl::task<gl::cmd_dispatch> dispatch_task;
	glm::u32vec2 extent;

public:
	hdr_compute_histogram_fragment(const gl::rendering_system &rs)
		: Base(rs,
			   "hdr_create_histogram.comp")
	{
		dispatch_task.attach_pipeline(pipeline());
	}
	~hdr_compute_histogram_fragment() noexcept {}

	hdr_compute_histogram_fragment(hdr_compute_histogram_fragment&&) = default;

	static lib::string name() { return "hdr_compute_histogram"; }

	void bind_buffers(const gl::array<gl::std430<std::uint32_t>> &histogram_data, 
					  const gl::array<hdr_bokeh_parameters> &hdr_bokeh_parameters_buffer) {
		pipeline()["histogram_data"] = gl::bind(histogram_data);
		pipeline()["hdr_bokeh_parameters_buffer"] = gl::bind(hdr_bokeh_parameters_buffer);
	}
	void set_source(const gl::pipeline::combined_image_sampler &hdr_lums,
					const glm::u32vec2 &extent) {
		pipeline()["hdr_lums"] = gl::bind(hdr_lums);
		this->extent = extent;
	}

	void record(gl::command_recorder &recorder) override final {
		const std::uint32_t workgroup = 32;
		const auto jobs = (extent + glm::u32vec2(workgroup - 1)) / workgroup;

		recorder << dispatch_task(jobs.x, jobs.y, 1);
	}
};

}
}
