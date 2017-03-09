//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>

#include <optional.hpp>

namespace StE {
namespace GL {

class vk_event {
private:
	optional<VkEvent> event;
	const vk_logical_device &device;

public:
	vk_event(const vk_logical_device &device) : device(device) {
		VkEventCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;

		VkEvent event;
		vk_result res = vkCreateEvent(device, &create_info, nullptr, &event);
		if (!res) {
			throw vk_exception(res);
		}

		this->event = event;
	}
	~vk_event() noexcept {
		destroy_event();
	}

	vk_event(vk_event &&) = default;
	vk_event &operator=(vk_event &&) = default;
	vk_event(const vk_event &) = delete;
	vk_event &operator=(const vk_event &) = delete;

	void destroy_event() {
		if (event) {
			vkDestroyEvent(device, *this, nullptr);
			event = none;
		}
	}

	/**
	*	@brief	Returns true if event is signaled
	*/
	bool is_signaled() const {
		vk_result res = vkGetEventStatus(device, *this);
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
		vk_result res = vkSetEvent(device, *this);
		if (!res) {
			throw vk_exception(res);
		}
	}
	/**
	*	@brief	Resets the event, setting its status to unsignaled
	*/
	void reset() const {
		vk_result res = vkResetEvent(device, *this);
		if (!res) {
			throw vk_exception(res);
		}
	}

	auto& get_creating_device() const { return device; }
	auto& get_event() const { return event.get(); }

	operator VkEvent() const { return get_event(); }
};

}
}
