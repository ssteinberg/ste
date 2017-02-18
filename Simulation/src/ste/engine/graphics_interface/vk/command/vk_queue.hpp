//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>

#include <optional.hpp>

namespace StE {
namespace GL {

class vk_queue {
private:
	optional<VkQueue> queue;

public:
	vk_queue(const vk_logical_device &device, std::uint32_t queue_family, std::uint32_t queue_index) {
		VkQueue q;
		vkGetDeviceQueue(device, queue_family, queue_index, &q);

		this->queue = q;
	}
	~vk_queue() noexcept {}

	vk_queue(vk_queue &&) = default;
	vk_queue &operator=(vk_queue &&) = default;
	vk_queue(const vk_queue &) = default;
	vk_queue &operator=(const vk_queue &) = default;

	void wait_idle() const {
		vkQueueWaitIdle(*this);
	}

	auto& get_queue() const { return queue.get(); }

	operator VkQueue() const { return get_queue(); }
};

}
}
