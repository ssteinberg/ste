//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_command_buffers.hpp>

#include <ste_resource_pool.hpp>

#include <optional.hpp>
#include <vector>
#include <allow_class_decay.hpp>

namespace StE {
namespace GL {

class vk_command_pool : 
	public ste_resource_pool_resetable_trait<const vk_logical_device &, std::uint32_t, VkCommandPoolCreateFlags>,
	public allow_class_decay<vk_command_pool, VkCommandPool>
{
private:
	optional<VkCommandPool> pool;
	const vk_logical_device &device;

public:
	vk_command_pool(const vk_logical_device &device, 
					std::uint32_t queue_family,
					VkCommandPoolCreateFlags flags = 0) : device(device) {
		VkCommandPoolCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = flags;
		create_info.queueFamilyIndex = queue_family;

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

	auto allocate_buffers(std::uint32_t count,
						  const vk_command_buffer_type &type) const {
		assert(count > 0);

		VkCommandBufferAllocateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		create_info.pNext = nullptr;
		create_info.commandPool = *this;
		create_info.commandBufferCount = count;
		type == vk_command_buffer_type::primary ?
			create_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY :
			create_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

		std::vector<vk_command_buffer> buffers;
		buffers.resize(count);
		vk_result res = vkAllocateCommandBuffers(device, &create_info, reinterpret_cast<VkCommandBuffer*>(&buffers[0]));
		if (!res) {
			throw vk_exception(res);
		}

		return vk_command_buffers(std::move(buffers), device, *this, type);
	}

	void reset() override {
		vk_result res = vkResetCommandPool(device, *this, 0);
		if (!res) {
			throw vk_exception(res);
		}
	}

	void reset_release() {
		vk_result res = vkResetCommandPool(device, *this, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		if (!res) {
			throw vk_exception(res);
		}
	}

	auto& get() const { return pool.get(); }
};

}
}
