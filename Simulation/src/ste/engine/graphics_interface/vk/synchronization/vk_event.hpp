//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_host_allocator.hpp>
#include <vk_logical_device.hpp>

#include <ste_resource_pool_traits.hpp>

#include <optional.hpp>
#include <alias.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_event : public allow_type_decay<vk_event<host_allocator>, VkEvent>, public ste_resource_pool_resetable_trait<const vk_logical_device<host_allocator> &> {
private:
	optional<VkEvent> event;
	alias<const vk_logical_device<host_allocator>> device;

public:
	vk_event(const vk_logical_device<host_allocator> &device) : device(device) {
		VkEventCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;

		VkEvent event;
		vk_result res = vkCreateEvent(device, &create_info, &host_allocator::allocation_callbacks(), &event);
		if (!res) {
			throw vk_exception(res);
		}

		this->event = event;
	}
	~vk_event() noexcept {
		destroy_event();
	}

	vk_event(vk_event &&) = default;
	vk_event &operator=(vk_event &&o) noexcept {
		destroy_event();

		event = std::move(o.event);
		device = std::move(o.device);

		return *this;
	}
	vk_event(const vk_event &) = delete;
	vk_event &operator=(const vk_event &) = delete;

	void destroy_event() {
		if (event) {
			vkDestroyEvent(device.get(), *this, &host_allocator::allocation_callbacks());
			event = none;
		}
	}

	/**
	*	@brief	Returns true if event is signaled
	*/
	bool is_signaled() const {
		vk_result res = vkGetEventStatus(device.get(), *this);
		if (res != VK_EVENT_SET &&
			res != VK_EVENT_RESET) {
			// Returned error. Throw...
			throw vk_exception(res);
		}
		return res == VK_EVENT_SET;
	}
	/**
	*	@brief	Sets the event, setting its status to signaled
	*/
	void set() const {
		vk_result res = vkSetEvent(device.get(), *this);
		if (!res) {
			throw vk_exception(res);
		}
	}
	/**
	*	@brief	Resets the event, setting its status to unsignaled
	*/
	void reset() override {
		vk_result res = vkResetEvent(device.get(), *this);
		if (!res) {
			throw vk_exception(res);
		}
	}

	auto& get_creating_device() const { return device.get(); }
	auto& get() const { return event.get(); }
};

}

}
}
