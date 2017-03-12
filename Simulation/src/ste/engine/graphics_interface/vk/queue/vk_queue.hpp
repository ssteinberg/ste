//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_command_buffers.hpp>
#include <vk_semaphore.hpp>
#include <vk_fence.hpp>

#include <optional.hpp>

#include <vector>
#include <functional>

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

	/**
	*	@brief	Blocks the caller till the queue has finished processing all the work on the device
	*/
	void wait_idle() const {
		vkQueueWaitIdle(*this);
	}

	/**
	*	@brief	Submits one or more command buffers for execution on the queue
	*	
	*	@param	command_buffers		Command buffers to submit
	*	@param	wait_semaphores		Array of pairs of semaphores upon which to wait before execution, and corresponsing pipeline
	*								stages at which the wait occurs
	*	@param	signal_semaphores	Sempahores to signal once the commands have completed execution
	*	@param	fence				Optional fence, to be signaled when the commands have completed execution
	*/
	void submit(const std::vector<vk_command_buffer*> &command_buffers,
				const std::vector<std::pair<vk_semaphore*, VkPipelineStageFlags>> &wait_semaphores,
				const std::vector<vk_semaphore*> &signal_semaphores,
				const vk_fence *fence = nullptr) const {
		std::vector<VkCommandBuffer> cb;
		std::vector<VkSemaphore> wait;
		std::vector<VkPipelineStageFlags> stages;
		std::vector<VkSemaphore> signal;

		cb.resize(command_buffers.size());
		for (int i=0;i<command_buffers.size();++i)
			cb[i] = *command_buffers[i];

		wait.resize(wait_semaphores.size());
		stages.resize(wait_semaphores.size());
		for (int i = 0; i<wait_semaphores.size(); ++i) {
			wait[i] = *wait_semaphores[i].first;
			stages[i] = wait_semaphores[i].second;
		}

		signal.resize(signal_semaphores.size());
		for (int i = 0; i<signal_semaphores.size(); ++i)
			signal[i] = *signal_semaphores[i];

		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.pNext = nullptr;
		info.waitSemaphoreCount = wait.size();
		info.pWaitSemaphores = wait.data();
		info.pWaitDstStageMask = stages.data();
		info.commandBufferCount = cb.size();
		info.pCommandBuffers = cb.data();
		info.signalSemaphoreCount = signal.size();
		info.pSignalSemaphores = signal.data();

		vk_result res = vkQueueSubmit(*this,
									  1, &info,
									  fence != nullptr ? *fence : VK_NULL_HANDLE);
		if (!res) {
			throw vk_exception(res);
		}
	}

	auto& get_queue() const { return queue.get(); }
	auto get_queue_family_index() const { return queue_family; }

	operator VkQueue() const { return get_queue(); }
};

}
}
