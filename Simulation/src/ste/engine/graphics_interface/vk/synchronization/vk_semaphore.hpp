//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_host_allocator.hpp>
#include <vk_ext_debug_marker.hpp>

#include <ste_resource_pool.hpp>

#include <optional.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_semaphore : public ste_resource_pool_const_trait<const vk_logical_device<host_allocator> &, const char*>, public allow_type_decay<vk_semaphore<host_allocator>, VkSemaphore> {
private:
	optional<VkSemaphore> semaphore;
	alias<const vk_logical_device<host_allocator>> device;

public:
	vk_semaphore(const vk_logical_device<host_allocator> &device,
				 const char *name) : device(device) {
		VkSemaphoreCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;

		VkSemaphore semaphore;
		const vk_result res = vkCreateSemaphore(device, &create_info, &host_allocator::allocation_callbacks(), &semaphore);
		if (!res) {
			throw vk_exception(res);
		}

		// Set object debug marker
		vk_debug_marker_set_object_name(device,
										semaphore,
										VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT,
										name);

		this->semaphore = semaphore;
	}
	~vk_semaphore() noexcept {
		destroy_semaphore();
	}

	vk_semaphore(vk_semaphore &&) = default;
	vk_semaphore &operator=(vk_semaphore &&o) noexcept {
		destroy_semaphore();

		semaphore = std::move(o.semaphore);
		device = std::move(o.device);

		return *this;
	}
	vk_semaphore(const vk_semaphore &) = delete;
	vk_semaphore &operator=(const vk_semaphore &) = delete;

	void destroy_semaphore() {
		if (semaphore) {
			vkDestroySemaphore(device.get(), *this, &host_allocator::allocation_callbacks());
			semaphore = none;
		}
	}

	auto& get_creating_device() const { return device.get(); }
	auto& get() const { return semaphore.get(); }
};

}

}
}
