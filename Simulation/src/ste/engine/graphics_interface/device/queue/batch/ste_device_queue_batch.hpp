//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_device_queue_batch_base.hpp>
#include <command_buffer.hpp>

#include <ste_device_queue_command_pool.hpp>

#include <lib/shared_ptr.hpp>
#include <lib/vector.hpp>
#include <type_traits>

namespace ste {
namespace gl {

/*
*	@brief	A common device batch.
*			Command buffers can be acquired from the supplied pool. Submitting the batch submits in turn the recorded command buffers.
*/
template <typename UserData = void>
class ste_device_queue_batch : public ste_device_queue_batch_user_data<UserData> {
	friend class ste_device_queue;
	using Base = ste_device_queue_batch_user_data<UserData>;

public:
	using command_buffer_t = command_buffer_primary<false>;
	using pool_t = ste_resource_pool<ste_device_queue_command_pool>::resource_t;

protected:
	using Base::ctor;
	using Base::fence_strong;

protected:
	pool_t pool;

protected:
	lib::vector<command_buffer_t> command_buffers;

	auto size() const { return command_buffers.size(); }
	auto begin() const { return command_buffers.begin(); }
	auto end() const { return command_buffers.end(); }

	void submit(const vk::vk_queue<> &q) const override final {
		// Create semaphore handles
		auto wait_semaphore_handles = lib::vector<vk::vk_queue<>::wait_semaphore_t>(Base::wait_semaphores.begin(),
																					Base::wait_semaphores.end());
		auto signal_semaphore_handles = Base::vk_semaphores(Base::signal_semaphores);

		// Copy command buffers' handles for submission and prepare dependecies
		lib::vector<vk::vk_command_buffer> command_buffers;
		command_buffers.reserve(size());
		for (auto &b : *this) {
			// Add command buffer handle
			command_buffers.push_back(static_cast<vk::vk_command_buffer>(b));
			// Move dependencies
			for (auto &&d : b.dependencies)
				wait_semaphore_handles.emplace_back(std::move(d));
		}

		// Submit finalized buffers
		q.submit(command_buffers,
				 wait_semaphore_handles,
				 signal_semaphore_handles,
				 &(*fence_strong)->get_fence());

		// Signal fence's host-side future
		(*fence_strong)->signal();
	}

public:
	template <typename S = UserData, typename... UserDataArgs>
	ste_device_queue_batch(std::enable_if_t<!std::is_void_v<S>, ctor>,
						   std::uint32_t queue_index,
						   pool_t &&pool,
						   typename Base::fence_ptr_strong_t &&fence,
						   UserDataArgs &&... user_data_args)
		: Base(queue_index,
			   std::move(fence),
			   std::forward<UserDataArgs>(user_data_args)...),
		  pool(std::move(pool)) {}
	template <typename S = UserData>
	ste_device_queue_batch(std::enable_if_t<std::is_void_v<S>, ctor>,
						   std::uint32_t queue_index,
						   pool_t &&pool,
						   typename Base::fence_ptr_strong_t &&fence)
		: Base(queue_index,
			   std::move(fence)),
		pool(std::move(pool)) {}

	virtual ~ste_device_queue_batch() noexcept {}

	ste_device_queue_batch(ste_device_queue_batch &&) = default;
	ste_device_queue_batch &operator=(ste_device_queue_batch &&) = default;

	/*
	 *	@brief	Creates a new command buffer owned by the batch
	 */
	auto &acquire_command_buffer() {
		const command_pool &p = pool;

		command_buffer_t buffer = p.allocate_primary_buffer();
		command_buffers.emplace_back(std::move(buffer));

		return command_buffers.back();
	}

	auto &get_queue_descriptor() const { return pool.get().get().get_queue_descriptor(); }
};

}
}
