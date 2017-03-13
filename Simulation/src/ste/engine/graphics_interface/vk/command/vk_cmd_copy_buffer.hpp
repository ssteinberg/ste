//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_command.hpp>
#include <vk_buffer.hpp>
#include <vk_image.hpp>

#include <vector>

namespace StE {
namespace GL {

class vk_cmd_copy_buffer : public vk_command {
private:
	VkBuffer src_buffer;
	VkBuffer dst_buffer;

	std::vector<VkBufferCopy> ranges;

public:
	template <typename T1, bool sparse1, typename T2, bool sparse2>
	vk_cmd_copy_buffer(const vk_buffer<T1, sparse1> &src_buffer,
					   const vk_buffer<T2, sparse2> &dst_buffer,
					   const std::vector<VkBufferCopy> &ranges = {})
		: src_buffer(src_buffer), dst_buffer(dst_buffer), ranges(ranges)
	{
		if (this->ranges.size() == 0) {
			VkBufferCopy c = {
				0, 0,
				std::min(src_buffer.get_elements_count() * sizeof(T1), dst_buffer.get_elements_count() * sizeof(T2))
			};
			this->ranges.push_back(c);
		}
	}
	virtual ~vk_cmd_copy_buffer() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		vkCmdCopyBuffer(command_buffer, src_buffer,
						dst_buffer,
						ranges.size(), ranges.data());
	}
};

}
}
