// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fragment_compute.hpp>

namespace ste {
namespace graphics {

class hdr_compute_histogram_fragment : public gl::fragment_compute {
	using Base = gl::fragment_compute;

public:
	hdr_compute_histogram_fragment(const gl::rendering_system &rs)
		: Base(rs,
			   "hdr_create_histogram.comp")
	{}
	~hdr_compute_histogram_fragment() noexcept {}

	static const std::string& name() { return "hdr_compute_histogram"; }

	void bind_buffers(const gl::device_buffer_base &histogram_data, const gl::device_buffer_base &hdr_bokeh_parameters_buffer) {
		pipeline["histogram_data"] = gl::bind(histogram_data);
		pipeline["hdr_bokeh_parameters_buffer"] = gl::bind(hdr_bokeh_parameters_buffer);
	}

	void record(gl::command_recorder &recorder) override final {

	}
};

}
}
