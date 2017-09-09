// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_compute.hpp>
#include <array.hpp>

#include <hdr_dof_bokeh_parameters.hpp>

#include <task.hpp>
#include <cmd_dispatch.hpp>

namespace ste {
namespace graphics {

class hdr_compute_histogram_sums_fragment : public gl::fragment_compute {
	using Base = gl::fragment_compute;

	gl::task<gl::cmd_dispatch> dispatch_task;

public:
	hdr_compute_histogram_sums_fragment(const gl::rendering_system &rs)
		: Base(rs,
			   "hdr_compute_histogram_sums.comp")
	{
		dispatch_task.attach_pipeline(pipeline());
	}
	~hdr_compute_histogram_sums_fragment() noexcept {}

	hdr_compute_histogram_sums_fragment(hdr_compute_histogram_sums_fragment&&) = default;

	void bind_buffers(const gl::array<gl::std430<std::uint32_t>> &histogram_sums,
					  const gl::array<gl::std430<std::uint32_t>> &histogram_bins,
					  const gl::array<hdr_bokeh_parameters> &hdr_bokeh_parameters_buffer) {
		pipeline()["histogram_sums"] = gl::bind(histogram_sums);
		pipeline()["histogram_bins"] = gl::bind(histogram_bins);
		pipeline()["hdr_bokeh_parameters_buffer"] = gl::bind(hdr_bokeh_parameters_buffer);
	}
	void set_source(const glm::u32vec2 &extent) {
		pipeline()["hdr_lum_resolution_t.hdr_lum_resolution"] = (extent.x / 32) * (extent.y / 32);
	}

	static const lib::string& name() { return "hdr_compute_histogram_sums"; }

	void record(gl::command_recorder &recorder) override final {
		recorder << dispatch_task(1, 1, 1);
	}
};

}
}
