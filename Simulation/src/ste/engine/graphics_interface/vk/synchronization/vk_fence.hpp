//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>

#include <optional.hpp>

#include <chrono>
#include <vector>

namespace StE {
namespace GL {

class vk_fence {
private:
	optional<VkFence> fence;
	const vk_logical_device &device;

public:
	vk_fence(const vk_logical_device &device, 
			 bool signaled = false) : device(device) {
		VkFenceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

		VkFence fence;
		vk_result res = vkCreateFence(device, &create_info, nullptr, &fence);
		if (!res) {
			throw vk_exception(res);
		}

		this->fence = fence;
	}
	~vk_fence() noexcept {
		destroy_fence();
	}

	vk_fence(vk_fence &&) = default;
	vk_fence &operator=(vk_fence &&) = default;
	vk_fence(const vk_fence &) = delete;
	vk_fence &operator=(const vk_fence &) = delete;

	void destroy_fence() {
		if (fence) {
			vkDestroyFence(device, *this, nullptr);
			fence = none;
		}
	}

	/**
	*	@brief	Returns true if fence is signaled
	*/
	bool is_signaled() const {
		vk_result res = vkGetFenceStatus(device, *this);
		if (res != VK_SUCCESS &&
			res != VK_NOT_READY) {
			// Returned error. Throw...
			throw vk_exception(res);
		}
		return res == VK_SUCCESS;
	}
	/**
	*	@brief	Resets the fence, setting its status to unsignaled
	*/
	void reset() const {
		vk_result res = vkResetFences(device, 1, &fence.get());
		if (!res) {
			throw vk_exception(res);
		}
	}
	/**
	*	@brief	Waits for fence to become signaled or until timeout has passed.
	*
	*	@param timeout	Maximal time to wait
	*	
	*	@return	Returns true if signaled, false on timeout.
	*/
	template <class Rep, class Period>
	bool wait_idle(const std::chrono::duration<Rep, Period> &timeout) const {
		std::uint64_t timeout_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();
		return vkWaitForFences(device,
							   1,
							   &fence.get(),
							   true,
							   timeout_ns) == VK_SUCCESS;
	}

	auto& get_creating_device() const { return device; }
	auto& get_fence() const { return fence.get(); }

	operator VkFence() const { return get_fence(); }
};

/**
*	@brief	Waits for all the fences to become signaled or until timeout has passed.
*
*	@param fences	Fences to wait for
*	@param timeout	Maximal time to wait
*	
*	@return	Returns true if signaled, false on timeout.
*/
template <class Rep, class Period>
bool inline vk_fence_wait_all(const std::vector<vk_fence*> &fences,
							  const std::chrono::duration<Rep, Period> &timeout) {
	assert(fences.size());

	std::uint64_t timeout_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();
	std::vector<VkFence> v(fences.size());
	for (int i = 0; i < fences.size(); ++i)
		v[i] = *fences[i];

	return vkWaitForFences((*fences.begin())->get_creating_device(),
						   v.size(),
						   v.data(),
						   true,
						   timeout_ns) == VK_SUCCESS;
}

/**
*	@brief	Waits for any of the fences to become signaled or until timeout has passed.
*
*	@param fences	Fences to wait for
*	@param timeout	Maximal time to wait
*
*	@return	Returns true if signaled, false on timeout.
*/
template <class Rep, class Period>
bool inline vk_fence_wait_any(const std::vector<vk_fence*> &fences,
							  const std::chrono::duration<Rep, Period> &timeout) {
	assert(fences.size());

	std::uint64_t timeout_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();
	std::vector<VkFence> v(fences.size());
	for (int i = 0; i < fences.size(); ++i)
		v[i] = *fences[i];

	return vkWaitForFences((*fences.begin())->get_creating_device(),
						   v.size(),
						   v.data(),
						   false,
						   timeout_ns) == VK_SUCCESS;
}

}
}
