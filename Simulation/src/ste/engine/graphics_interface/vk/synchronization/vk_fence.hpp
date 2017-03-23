//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>

#include <ste_resource_pool_traits.hpp>

#include <optional.hpp>

#include <chrono>
#include <vector>
#include <limits>

namespace StE {
namespace GL {

class vk_fence : public ste_resource_pool_resetable_trait<const vk_logical_device &, bool> {
private:
	optional<VkFence> fence;
	std::reference_wrapper<const vk_logical_device> device;

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
			vkDestroyFence(device.get(), *this, nullptr);
			fence = none;
		}
	}

	/**
	*	@brief	Returns true if fence is signaled
	*/
	bool is_signaled() const {
		vk_result res = vkGetFenceStatus(device.get(), *this);
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
	void reset() override {
		vk_result res = vkResetFences(device.get(), 1, &fence.get());
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
	template <class Rep = std::chrono::nanoseconds::rep, class Period = std::chrono::nanoseconds::period>
	bool wait_idle(const std::chrono::duration<Rep, Period> &timeout = std::chrono::nanoseconds(std::numeric_limits<uint64_t>::max())) const {
		std::uint64_t timeout_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();
		return vkWaitForFences(device.get(),
							   1,
							   &fence.get(),
							   true,
							   timeout_ns) == VK_SUCCESS;
	}

	auto& get_creating_device() const { return device.get(); }
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
template <class Rep = std::chrono::nanoseconds::rep, class Period = std::chrono::nanoseconds::period>
bool inline vk_fence_wait_all(const std::vector<std::reference_wrapper<const vk_fence>> &fences,
							  const std::chrono::duration<Rep, Period> &timeout = std::chrono::nanoseconds(std::numeric_limits<uint64_t>::max())) {
	assert(fences.size());

	std::uint64_t timeout_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();
	std::vector<VkFence> v(fences.size());
	for (int i = 0; i < fences.size(); ++i)
		v[i] = fences[i].get();

	return vkWaitForFences((*fences.begin()).get().get_creating_device(),
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
template <class Rep = std::chrono::nanoseconds::rep, class Period = std::chrono::nanoseconds::period>
bool inline vk_fence_wait_any(const std::vector<std::reference_wrapper<const vk_fence>> &fences,
							  const std::chrono::duration<Rep, Period> &timeout = std::chrono::nanoseconds(std::numeric_limits<uint64_t>::max())) {
	assert(fences.size());

	std::uint64_t timeout_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();
	std::vector<VkFence> v(fences.size());
	for (int i = 0; i < fences.size(); ++i)
		v[i] = fences[i].get();

	return vkWaitForFences((*fences.begin()).get().get_creating_device(),
						   v.size(),
						   v.data(),
						   false,
						   timeout_ns) == VK_SUCCESS;
}

}
}
