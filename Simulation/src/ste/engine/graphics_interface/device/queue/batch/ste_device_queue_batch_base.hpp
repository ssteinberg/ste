//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <shared_fence.hpp>
#include <vk_queue.hpp>

#include <semaphore.hpp>
#include <wait_semaphore.hpp>

#include <lib/unique_ptr.hpp>
#include <lib/shared_ptr.hpp>
#include <lib/vector.hpp>
#include <type_traits>

namespace ste {
namespace gl {

class ste_device_queue;

/*
*	@brief	A device batch facilitates creating command buffers, recording commands into command buffers and submission of said command buffers to a device queue.
*/
class ste_device_queue_batch_base {
	friend ste_device_queue;

public:
	using fence_t = ste_resource_pool<shared_fence<void>>::resource_t;
	using fence_ptr_strong_t = lib::shared_ptr<fence_t>;

private:
	// ste_device_queue sets this on submit
	mutable bool submitted{ false };

protected:
	struct ctor {};

	std::uint32_t queue_index;

	fence_ptr_strong_t fence_strong;

public:
	/*
	*	@brief	Array of pairs of semaphores upon which to wait before execution, and corresponsing pipeline
	*			stages at which the wait occurs.
	*/
	lib::vector<wait_semaphore> wait_semaphores;

	/*
	*	@brief	Sempahores to signal once the commands have completed execution.
	*/
	lib::vector<const semaphore*> signal_semaphores;

protected:
	/*
	*	@brief	Converts a vector of semaphores to a vector of Vulkan semaphore handles
	*/
	static auto vk_semaphores(const lib::vector<const semaphore*> &s) {
		lib::vector<VkSemaphore> vk_sems;
		vk_sems.reserve(s.size());
		for (auto &sem : s)
			vk_sems.push_back(*sem);

		return vk_sems;
	}

	/*
	*	@brief	Should submit the batch to the supplied queue and signal fence's host-side status after submission.
	*/
	virtual void submit(const vk::vk_queue<> &) const = 0;

public:
	ste_device_queue_batch_base(std::uint32_t queue_index,
								fence_ptr_strong_t &&fence)
		: queue_index(queue_index),
		fence_strong(std::move(fence)) {}

	virtual ~ste_device_queue_batch_base() noexcept {
		assert(submitted && "Batch created but not submitted!");
	}

	ste_device_queue_batch_base(ste_device_queue_batch_base &&) = default;
	ste_device_queue_batch_base &operator=(ste_device_queue_batch_base &&) = default;

	/*
	*	@brief	Returns a shared pointer to the fence object.
	*			The fence will be signalled once the batch have completed execution.
	*/
	const auto &get_fence_ptr() const {
		return fence_strong;
	}

	/*
	*	@brief	Checks fence, device and host status, for batch completion.
	*/
	bool is_batch_complete() const {
		return (*fence_strong)->is_signaled();
	}

	auto get_queue_index() const { return queue_index; }
};

namespace _detail {

template <typename T>
struct ste_device_queue_batch_user_data_t {
	lib::unique_ptr<T> user_data;

	template <typename... UserDataArgs>
	ste_device_queue_batch_user_data_t(UserDataArgs &&... user_data_args)
		: user_data(lib::allocate_unique<T>(std::forward<UserDataArgs>(user_data_args)...)) {}
};
template <>
struct ste_device_queue_batch_user_data_t<void> {};

}

/*
*	@brief	A device command batch virtual class.
*			Allows taking ownership of user data for the batch's lifetime.
*/
template <typename UserData = void>
class ste_device_queue_batch_user_data : public ste_device_queue_batch_base {
	using Base = ste_device_queue_batch_base;

private:
	_detail::ste_device_queue_batch_user_data_t<UserData> user_data_wrap;

public:
	template <typename S = UserData, typename... UserDataArgs>
	ste_device_queue_batch_user_data(std::enable_if_t<!std::is_void_v<S>, std::uint32_t> queue_index,
									 fence_ptr_strong_t &&fence,
									 UserDataArgs &&... user_data_args)
		: Base(queue_index,
			   std::move(fence)),
		user_data_wrap(std::forward<UserDataArgs>(user_data_args)...) {}

	template <typename S = UserData>
	ste_device_queue_batch_user_data(std::enable_if_t<std::is_void_v<S>, std::uint32_t> queue_index,
									 fence_ptr_strong_t &&fence)
		: Base(queue_index,
			   std::move(fence)) {}

	virtual ~ste_device_queue_batch_user_data() noexcept {}

	ste_device_queue_batch_user_data(ste_device_queue_batch_user_data &&) = default;
	ste_device_queue_batch_user_data &operator=(ste_device_queue_batch_user_data &&) = default;

	template <typename S = UserData>
	std::enable_if_t<!std::is_void_v<S>, S&> user_data() { return *user_data_wrap.user_data; }

	template <typename S = UserData>
	std::enable_if_t<!std::is_void_v<S>, const S&> user_data() const { return *user_data_wrap.user_data; }
};

}
}
