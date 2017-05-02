//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

namespace ste {
namespace gl {

class command_buffer;

class command {
	friend class command_recorder;

private:
	virtual void operator()(const command_buffer &command_buffer, command_recorder &) const = 0;

public:
	command() = default;
	virtual ~command() noexcept {}
};

}
}
