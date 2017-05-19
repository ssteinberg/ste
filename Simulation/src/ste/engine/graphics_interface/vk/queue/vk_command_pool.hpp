//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <ste_queue_family.hpp>
#include <vk_command_buffers.hpp>

#include <ste_resource_pool.hpp>

#include <optional.hpp>
#include <lib/vector.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

namespace vk {

class vk_command_pool :
	public ste_resource_pool_resetable_trait<const vk_logical_device &, std::uint32_t, VkCommandPoolCreateFlags>,
	public allow_type_decay<vk_command_pool, VkCommandPool>
{
private:
	optional<VkCommandPool> pool;
	std::reference_wrapper<const vk_logical_device> device;

public:
	vk_command_pool(const vk_logical_device &device,
					const ste_queue_family &queue_family,
					VkCommandPoolCreateFlags flags = 0) : device(device) {
		VkCommandPoolCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = flags;
		create_info.queueFamilyIndex = static_cast<std::uint32_t>(queue_family);

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
			vkDestroyCommandPool(device.get(), *this, nullptr);
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

		lib::vector<VkCommandBuffer> buffers;
		buffers.resize(count);
		vk_result res = vkAllocateCommandBuffers(device.get(), &create_info, &buffers[0]);
		if (!res) {
			throw vk_exception(res);
		}

		lib::vector<vk_command_buffer> command_buffers;
		command_buffers.reserve(count);
		for (auto &b : buffers)
			command_buffers.emplace_back(vk_command_buffer{ b });

		return vk_command_buffers(std::move(command_buffers), device.get(), *this, type);
	}

	void reset() override {
		vk_result res = vkResetCommandPool(device.get(), *this, 0);
		if (!res) {
			throw vk_exception(res);
		}
	}

	void reset_release() {
		vk_result res = vkResetCommandPool(device.get(), *this, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		if (!res) {
			throw vk_exception(res);
		}
	}

	void trim() {
		vkTrimCommandPoolKHR(device.get(), *this, 0);
	}

	auto& get() const { return pool.get(); }
};

}

}
}
