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

class hdr_adaptation_fragment : public gl::fragment_compute<hdr_adaptation_fragment> {
	using Base = gl::fragment_compute<hdr_adaptation_fragment>;

	gl::task<gl::cmd_dispatch> dispatch_task;
	glm::u32vec2 extent;

public:
	hdr_adaptation_fragment(const gl::rendering_system &rs)
		: Base(rs,
			   "hdr_adaptation.comp") {
		dispatch_task.attach_pipeline(pipeline());
	}

	~hdr_adaptation_fragment() noexcept {}

	hdr_adaptation_fragment(hdr_adaptation_fragment &&) = default;

	static lib::string name() { return "hdr_adaptation_fragment"; }

	void bind_buffers(const gl::array<hdr_bokeh_parameters> &hdr_bokeh_parameters_buffer,
					  const gl::array<hdr_bokeh_parameters> &hdr_bokeh_parameters_buffer_prev) {
		pipeline()["hdr_bokeh_parameters_buffer"] = gl::bind(hdr_bokeh_parameters_buffer);
		pipeline()["hdr_bokeh_parameters_buffer_prev"] = gl::bind(hdr_bokeh_parameters_buffer_prev);
	}

	void set_tick_time_ms(float time_ms) {
		pipeline()["time_push_t.time_ms"] = time_ms;
	}

	void record(gl::command_recorder &recorder) override final {
		recorder << dispatch_task(1, 1, 1);
	}
};

}
}
