//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vk_command_buffers.hpp>

namespace StE {
namespace GL {

class vk_command {
	friend class vk_command_recorder;

protected:
	static void execute(const vk_command_buffer &command_buffer, const vk_command &cmd) {
		cmd(command_buffer);
	}

private:
	virtual void operator()(const vk_command_buffer &) const { 
		assert(false && "One of the operator() method must be overriden");
	}
	virtual void operator()(const vk_command_buffer &command_buffer, vk_command_recorder &) const {
		return (*this)(command_buffer);
	}

public:
	virtual ~vk_command() noexcept {}
};

}
}
