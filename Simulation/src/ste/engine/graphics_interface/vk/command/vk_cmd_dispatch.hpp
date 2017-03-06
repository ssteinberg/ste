//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>

namespace StE {
namespace GL {

class vk_cmd_dispatch : public vk_command {
private:
	std::uint32_t x, y, z;

public:
	vk_cmd_dispatch(std::uint32_t x, 
					std::uint32_t y = 1, 
					std::uint32_t z = 1) : x(x), y(y), z(z)
	{}
	virtual ~vk_cmd_dispatch() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdDispatch(command_buffer, x, y, z);
	}
};

}
}
