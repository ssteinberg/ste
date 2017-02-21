//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>

namespace StE {
namespace GL {

class vk_command_pool {
private:
	VkCommandPool pool{ VK_NULL_HANDLE };
	const vk_logical_device &device;

public:
	vk_command_pool(const vk_logical_device &device, 
					std::uint32_t queue_family,
					bool transient = false) : device(device) {
		VkCommandPoolCreateInfo create_info;
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
		if (pool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(device, *this, nullptr);
			pool = VK_NULL_HANDLE;
		}
	}

	auto& get_command_pool() const { return pool; }

	operator VkCommandPool() const { return get_command_pool(); }
};

}
}
