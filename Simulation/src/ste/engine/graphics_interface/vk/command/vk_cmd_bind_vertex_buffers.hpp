//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_buffer.hpp>
#include <cassert>

#include <vector>

namespace StE {
namespace GL {

class vk_cmd_bind_vertex_buffers : public vk_command {
private:
	std::uint32_t first;
	std::vector<VkBuffer> buffers;
	std::vector<std::uint64_t> offsets;

public:
	template <typename T, bool sparse>
	vk_cmd_bind_vertex_buffers(std::uint32_t first_binding_index,
							   const vk_buffer<T, sparse> &buffer,
							   std::uint64_t offset = 0)
		: first(first_binding_index), buffers({ buffer }), offsets({ offset })
	{}
	vk_cmd_bind_vertex_buffers(std::uint32_t first_binding_index,
							   const std::vector<VkBuffer> &buffers,
							   const std::vector<std::uint64_t> &offsets) 
		: first(first_binding_index), buffers(buffers), offsets(offsets)
	{
		assert(buffers.size() == offsets.size());
	}
	virtual ~vk_cmd_bind_vertex_buffers() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdBindVertexBuffers(command_buffer, first, buffers.size(), buffers.data(), offsets.data());
	}
};

}
}
