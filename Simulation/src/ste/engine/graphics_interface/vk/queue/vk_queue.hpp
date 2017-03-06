//	StE
// © Shlomi Steinberg 2015-2017

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
	std::uint32_t queue_family;

public:
	vk_queue(const vk_logical_device &device, std::uint32_t queue_family, std::uint32_t queue_index) 
		: queue_family(queue_family)
	{
		VkQueue q;
		vkGetDeviceQueue(device, queue_family, queue_index, &q);

		this->queue = q;
	}
	~vk_queue() noexcept {}

	vk_queue(vk_queue &&) = default;
	vk_queue &operator=(vk_queue &&) = default;
	vk_queue(const vk_queue &) = delete;
	vk_queue &operator=(const vk_queue &) = delete;

	void wait_idle() const {
		vkQueueWaitIdle(*this);
	}

	auto& get_queue() const { return queue.get(); }
	auto get_queue_family_index() const { return queue_family; }

	operator VkQueue() const { return get_queue(); }
};

}
}
