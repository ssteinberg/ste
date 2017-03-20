//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <shared_fence.hpp>
#include <ste_device_queue_command_buffer.hpp>

#include <ste_device_queue_command_pool.hpp>
#include <shared_fence.hpp>

#include <memory>
#include <type_traits>
#include <function_traits.hpp>

namespace StE {
namespace GL {

namespace _detail {

template <typename Fence, typename Pool, typename UserData = void>
class ste_device_queue_batch_impl {
	friend class ste_device_queue;

public:
	using pool_t = Pool;
	using fence_t = Fence;

public:
	using fence_ptr_strong_t = std::shared_ptr<fence_t>;
	using fence_ptr_weak_t = std::weak_ptr<fence_t>;

private:
	using shared_fence_t = shared_fence<void>;

private:
	pool_t pool;
	std::uint32_t queue_index;

protected:
	fence_ptr_strong_t fence_strong;
	std::vector<ste_device_queue_command_buffer> command_buffers;

	auto begin() const { return std::begin(command_buffers); }
	auto end() const { return std::end(command_buffers); }

public:
	ste_device_queue_batch_impl(std::uint32_t queue_index,
								pool_t &&pool,
								const fence_ptr_strong_t &f)
		: pool(std::move(pool)),
		queue_index(queue_index),
		fence_strong(f)
	{}
	virtual ~ste_device_queue_batch_impl() noexcept {}

	ste_device_queue_batch_impl(ste_device_queue_batch_impl&&) = default;
	ste_device_queue_batch_impl &operator=(ste_device_queue_batch_impl&&) = default;

	auto& acquire_command_buffer() {
		command_buffers.emplace_back(pool->get_pool(), vk_command_buffer_type::primary);
		return command_buffers.back();
	}

	const auto& get_fence_ptr() const { return fence_strong; }

	auto& get_fence() const {
		return **fence_strong;
	}

	bool is_batch_complete() const {
		const auto& f = get_fence();
		return f.is_signaled();
	}
};

using ste_device_queue_batch_base = ste_device_queue_batch_impl<
	ste_resource_pool<shared_fence<void>>::resource_t,
	ste_resource_pool<ste_device_queue_command_pool>::resource_t
>;

template <typename T>
struct ste_device_queue_batch_user_data_t {
	std::unique_ptr<T> user_data;
	template <typename... UserDataArgs>
	ste_device_queue_batch_user_data_t(UserDataArgs&&... user_data_args)
		: user_data(std::make_unique<T>(std::forward<UserDataArgs>(user_data_args)...))
	{}
};
template <>
struct ste_device_queue_batch_user_data_t<void> {};

}

template <typename UserData = void>
class ste_device_queue_batch : public _detail::ste_device_queue_batch_base {
	using Base = _detail::ste_device_queue_batch_base;

private:
	_detail::ste_device_queue_batch_user_data_t<UserData> user_data_wrap;

public:
	template <typename S = UserData, typename... UserDataArgs>
	ste_device_queue_batch(std::enable_if_t<!std::is_void_v<S>, std::uint32_t> queue_index,
						   pool_t &&pool,
						   const fence_ptr_strong_t &f,
						   UserDataArgs&&... user_data_args)
		: Base(queue_index,
			   std::move(pool),
			   f),
		user_data_wrap(std::forward<UserDataArgs>(user_data_args)...)
	{}
	template <typename S = UserData>
	ste_device_queue_batch(std::enable_if_t<std::is_void_v<S>, std::uint32_t> queue_index,
						   pool_t &&pool,
						   const fence_ptr_strong_t &f)
		: Base(queue_index,
			   std::move(pool),
			   f)
	{}

	template <typename S = UserData>
	std::enable_if_t<!std::is_void_v<S>, S&> user_data() { return *user_data_wrap.user_data; }
	template <typename S = UserData>
	std::enable_if_t<!std::is_void_v<S>, const S&> user_data() const { return *user_data_wrap.user_data; }
};

}
}
