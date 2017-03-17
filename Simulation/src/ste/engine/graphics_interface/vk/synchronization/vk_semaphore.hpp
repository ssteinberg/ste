//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <ste_resource_pool.hpp>

#include <optional.hpp>

namespace StE {
namespace GL {

class vk_semaphore : public ste_resource_pool_const_trait<const vk_logical_device &> {
private:
	optional<VkSemaphore> semaphore;
	const vk_logical_device &device;

public:
	vk_semaphore(const vk_logical_device &device) : device(device) {
		VkSemaphoreCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;

		VkSemaphore semaphore;
		vk_result res = vkCreateSemaphore(device, &create_info, nullptr, &semaphore);
		if (!res) {
			throw vk_exception(res);
		}

		this->semaphore = semaphore;
	}
	~vk_semaphore() noexcept {
		destroy_semaphore();
	}

	vk_semaphore(vk_semaphore &&) = default;
	vk_semaphore &operator=(vk_semaphore &&) = default;
	vk_semaphore(const vk_semaphore &) = delete;
	vk_semaphore &operator=(const vk_semaphore &) = delete;

	void destroy_semaphore() {
		if (semaphore) {
			vkDestroySemaphore(device, *this, nullptr);
			semaphore = none;
		}
	}

	auto& get_creating_device() const { return device; }
	auto& get_semaphore() const { return semaphore.get(); }

	operator VkSemaphore() const { return get_semaphore(); }
};

}
}
