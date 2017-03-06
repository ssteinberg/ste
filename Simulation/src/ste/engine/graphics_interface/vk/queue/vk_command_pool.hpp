//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_command_buffers.hpp>

#include <optional.hpp>

#include <vector>

namespace StE {
namespace GL {

class vk_command_pool {
private:
	optional<VkCommandPool> pool;
	const vk_logical_device &device;

public:
	vk_command_pool(const vk_logical_device &device, 
					std::uint32_t queue_family,
					bool transient = false) : device(device) {
		VkCommandPoolCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		create_info.queueFamilyIndex = queue_family;

		if (transient)
			create_info.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		VkCommandPool pool;
		vk_result res = vkCreateCommandPool(device, &create_info, nullptr, &pool);
		if (!res) {
			throw vk_exception(res);
		}

		this->pool = pool;
	}
	~vk_command_pool() noexcept {
		destroy_command_pool();
	}

	vk_command_pool(vk_command_pool &&) = default;
	vk_command_pool &operator=(vk_command_pool &&) = default;
	vk_command_pool(const vk_command_pool &) = default;
	vk_command_pool &operator=(const vk_command_pool &) = default;

	void destroy_command_pool() {
		if (pool) {
			vkDestroyCommandPool(device, *this, nullptr);
			pool = none;
		}
	}

	auto allocate_buffers(std::uint32_t count) const {
		assert(count > 0);

		VkCommandBufferAllocateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		create_info.pNext = nullptr;
		create_info.commandPool = *this;
		create_info.commandBufferCount = count;
		create_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		std::vector<vk_command_buffer> buffers;
		buffers.resize(count);
		vk_result res = vkAllocateCommandBuffers(device, &create_info, reinterpret_cast<VkCommandBuffer*>(&buffers[0]));
		if (!res) {
			throw vk_exception(res);
		}

		return vk_command_buffers(std::move(buffers), device, *this);
	}

	auto& get_command_pool() const { return pool.get(); }

	operator VkCommandPool() const { return get_command_pool(); }
};

}
}
