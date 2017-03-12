//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>

#include <vector>

namespace StE {
namespace GL {

class vk_command_buffer {
private:
	VkCommandBuffer buffer{ VK_NULL_HANDLE };

public:
	void begin(const VkCommandBufferUsageFlags &flags = 0) {
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.pNext = nullptr;
		begin_info.pInheritanceInfo = nullptr;
		begin_info.flags = flags;

		vk_result res = vkBeginCommandBuffer(*this, &begin_info);
		if (!res) {
			throw vk_exception(res);
		}
	}
	void end() {
		vk_result res = vkEndCommandBuffer(*this);
		if (!res) {
			throw vk_exception(res);
		}
	}

	void reset() {
		vk_result res = vkResetCommandBuffer(*this, 0);
		if (!res) {
			throw vk_exception(res);
		}
	}
	void reset_release() {
		vk_result res = vkResetCommandBuffer(*this, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		if (!res) {
			throw vk_exception(res);
		}
	}

	operator VkCommandBuffer() const { return buffer; }
};

class vk_command_buffers {
	friend class vk_command_pool;

private:
	std::vector<vk_command_buffer> buffers;
	const vk_command_pool &pool;
	const vk_logical_device &device;

private:
	vk_command_buffers(std::vector<vk_command_buffer> &&buffers,
					   const vk_logical_device &device,
					   const vk_command_pool &pool)
		: buffers(std::move(buffers)), pool(pool), device(device)
	{}

public:
	~vk_command_buffers() noexcept {
		free();
	}

	vk_command_buffers(vk_command_buffers &&) = default;
	vk_command_buffers &operator=(vk_command_buffers &&) = default;
	vk_command_buffers(const vk_command_buffers &) = default;
	vk_command_buffers &operator=(const vk_command_buffers &) = default;

	void free();

	auto& operator[](std::size_t n) { return buffers[n]; }
	auto& operator[](std::size_t n) const { return buffers[n]; }

	auto size() const { return buffers.size(); }

	auto begin() const { return buffers.begin(); }
	auto begin() { return buffers.begin(); }
	auto end() const { return buffers.end(); }
	auto end() { return buffers.end(); }
};

}
}
