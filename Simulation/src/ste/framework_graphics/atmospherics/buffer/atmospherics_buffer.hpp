// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <atmospherics_descriptor.hpp>

#include <array.hpp>
#include <command_recorder.hpp>

namespace ste {
namespace graphics {

class atmospherics_buffer {
private:
	using descriptor = atmospherics_descriptor;
	using buffer_type = gl::array<descriptor::descriptor_data>;

private:
	buffer_type buffer;

public:
	atmospherics_buffer(const ste_context &ctx,
						const descriptor::Properties &p)
		: buffer(ctx,
				 lib::vector<descriptor::descriptor_data>{ descriptor(p).get() },
				 gl::buffer_usage::storage_buffer)
	{}

	void update_data(gl::command_recorder &recorder,
					 const descriptor::Properties &p) {
		auto d = descriptor(p);
		recorder << buffer.overwrite_cmd(0, d.get());
	}
};

}
}
