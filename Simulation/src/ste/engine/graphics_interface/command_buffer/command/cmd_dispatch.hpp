//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>

namespace ste {
namespace gl {

class cmd_dispatch : public command {
private:
	std::uint32_t x, y, z;

public:
	cmd_dispatch(cmd_dispatch &&) = default;
	cmd_dispatch(const cmd_dispatch&) = default;
	cmd_dispatch &operator=(cmd_dispatch &&) = default;
	cmd_dispatch &operator=(const cmd_dispatch&) = default;

	cmd_dispatch(std::uint32_t x,
				 std::uint32_t y = 1,
				 std::uint32_t z = 1) : x(x), y(y), z(z) {}

	virtual ~cmd_dispatch() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) && override final {
		vkCmdDispatch(command_buffer, x, y, z);
	}
};

}
}
