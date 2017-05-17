//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <command.hpp>
#include <device_buffer_base.hpp>
#include <cassert>

#include <vector>

namespace ste {
namespace gl {

class cmd_bind_vertex_buffers : public command {
private:
	std::uint32_t first;
	std::vector<VkBuffer> buffers;
	std::vector<std::uint64_t> offsets;

public:
	cmd_bind_vertex_buffers(std::uint32_t first_binding_index,
							const device_buffer_base &buffer,
							std::uint64_t offset = 0)
		: first(first_binding_index), buffers({ buffer.get_buffer_handle() }), offsets({ offset * buffer.get_element_size_bytes() })
	{}
	cmd_bind_vertex_buffers(std::uint32_t first_binding_index,
							const std::vector<std::reference_wrapper<const device_buffer_base>> &buffers,
							const std::vector<std::uint64_t> &offsets)
		: first(first_binding_index)
	{
		assert(buffers.size() == offsets.size());

		this->buffers.reserve(buffers.size());
		this->offsets.reserve(offsets.size());
		for (std::size_t i = 0; i < buffers.size(); ++i) {
			this->buffers.push_back(buffers[i].get().get_buffer_handle());
			this->offsets.push_back(offsets[i] * buffers[i].get().get_element_size_bytes());
		}
	}
	virtual ~cmd_bind_vertex_buffers() noexcept {}

private:
	void operator()(const command_buffer &command_buffer, command_recorder &) const override final {
		vkCmdBindVertexBuffers(command_buffer, 
							   first, 
							   static_cast<std::uint32_t>(buffers.size()), 
							   buffers.data(),
							   offsets.data());
	}
};

}
}
