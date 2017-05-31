// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <atmospherics_descriptor.hpp>

#include <ring_buffer.hpp>
#include <command_recorder.hpp>
#include <pipeline_stage.hpp>

namespace ste {
namespace graphics {

class atmospherics_buffer {
private:
	using descriptor = atmospherics_descriptor;
	using buffer_type = gl::ring_buffer<descriptor::descriptor_data, 12>;

private:
	buffer_type buffer;
	std::uint32_t slot;

public:
	atmospherics_buffer(const descriptor::Properties &p) {
		update_data(p);
	}

	auto update_data(const descriptor::Properties &p) {
		auto d = descriptor(p);
		slot = buffer.commit(d.get());

		return slot;
	}

	void finish(gl::command_recorder &recorder,
				gl::pipeline_stage stage) const {
		buffer.signal(recorder,
					  slot,
					  stage);
	}
};

}
}
