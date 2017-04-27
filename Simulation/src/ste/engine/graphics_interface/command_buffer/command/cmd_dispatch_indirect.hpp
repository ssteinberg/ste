//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <vk_buffer.hpp>

namespace ste {
namespace gl {

class cmd_dispatch_indirect : public command {
private:
	std::reference_wrapper<const vk::vk_buffer> buffer;
	std::uint32_t offset;

public:
	cmd_dispatch_indirect(const vk::vk_buffer &buffer,
							 std::uint32_t offset = 0) 
		: buffer(buffer), offset(offset * buffer.get_element_size_bytes())
	{}
	virtual ~cmd_dispatch_indirect() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdDispatchIndirect(command_buffer, buffer.get(), offset);
	}
};

}
}
