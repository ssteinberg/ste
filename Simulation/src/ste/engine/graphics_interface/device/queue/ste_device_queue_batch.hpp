//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <shared_fence.hpp>
#include <command_buffer.hpp>

#include <ste_device_queue_command_pool.hpp>
#include <shared_fence.hpp>

#include <memory>
#include <type_traits>
#include <function_traits.hpp>

namespace ste {
namespace gl {

class ste_device_queue;

namespace _detail {

class ste_device_queue_batch_base {
	friend ste_device_queue;

private:
	std::uint32_t queue_index;
	mutable bool submitted{ false };

public:
	ste_device_queue_batch_base(std::uint32_t queue_index) : queue_index(queue_index) {}
	virtual ~ste_device_queue_batch_base() noexcept {
		assert(submitted && "Batch created but not submitted!");
	}

	ste_device_queue_batch_base(ste_device_queue_batch_base&&) = default;
	ste_device_queue_batch_base &operator=(ste_device_queue_batch_base&&) = default;

	virtual bool is_batch_complete() const = 0;
};

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

template <typename Pool, typename UserData = void>
class ste_device_queue_batch_impl : public ste_device_queue_batch_base {
public:
	using pool_t = Pool;

protected:
	pool_t pool;

private:
	ste_device_queue_batch_user_data_t<UserData> user_data_wrap;

public:
	template <typename S = UserData, typename... UserDataArgs>
	ste_device_queue_batch_impl(std::enable_if_t<!std::is_void_v<S>, std::uint32_t> queue_index,
								pool_t &&pool,
								UserDataArgs&&... user_data_args)
		: ste_device_queue_batch_base(queue_index),
		pool(std::move(pool)),
		user_data_wrap(std::forward<UserDataArgs>(user_data_args)...)
	{}
	template <typename S = UserData>
	ste_device_queue_batch_impl(std::enable_if_t<std::is_void_v<S>, std::uint32_t> queue_index,
								pool_t &&pool)
		: ste_device_queue_batch_base(queue_index),
		pool(std::move(pool))
	{}
	virtual ~ste_device_queue_batch_impl() noexcept {}

	ste_device_queue_batch_impl(ste_device_queue_batch_impl&&) = default;
	ste_device_queue_batch_impl &operator=(ste_device_queue_batch_impl&&) = default;

	template <typename S = UserData>
	std::enable_if_t<!std::is_void_v<S>, S&> user_data() { return *user_data_wrap.user_data; }
	template <typename S = UserData>
	std::enable_if_t<!std::is_void_v<S>, const S&> user_data() const { return *user_data_wrap.user_data; }
};

template <typename UserData>
using ste_device_queue_batch_pool = _detail::ste_device_queue_batch_impl<
	typename ste_resource_pool<ste_device_queue_command_pool>::resource_t,
	UserData
>;

}

template <typename UserData = void>
class ste_device_queue_batch_oneshot : public _detail::ste_device_queue_batch_pool<UserData> {
	friend class ste_device_queue;

	using Base = _detail::ste_device_queue_batch_pool<UserData>;

public:
	using fence_t = ste_resource_pool<shared_fence<void>>::resource_t;
	using fence_ptr_strong_t = std::shared_ptr<fence_t>;
	using fence_ptr_weak_t = std::weak_ptr<fence_t>;
	using shared_fence_t = shared_fence<void>;

	using command_buffer_t = command_buffer_primary<false>;

protected:
	fence_ptr_strong_t fence_strong;

protected:
	std::vector<command_buffer_t> command_buffers;

	auto begin() const { return command_buffers.begin(); }
	auto end() const { return command_buffers.end(); }

public:
	template <typename S = UserData, typename... UserDataArgs>
	ste_device_queue_batch_oneshot(std::enable_if_t<!std::is_void_v<S>, std::uint32_t> queue_index,
						   typename Base::pool_t &&pool,
						   const fence_ptr_strong_t &f,
						   UserDataArgs&&... user_data_args)
		: Base(queue_index, std::move(pool), std::forward<UserDataArgs>(user_data_args)...),
		fence_strong(f)
	{}
	template <typename S = UserData>
	ste_device_queue_batch_oneshot(std::enable_if_t<std::is_void_v<S>, std::uint32_t> queue_index,
						   typename Base::pool_t &&pool,
						   const fence_ptr_strong_t &f)
		: Base(queue_index, std::move(pool)),
		fence_strong(f)
	{}
	virtual ~ste_device_queue_batch_oneshot() noexcept {}

	ste_device_queue_batch_oneshot(ste_device_queue_batch_oneshot&&) = default;
	ste_device_queue_batch_oneshot &operator=(ste_device_queue_batch_oneshot&&) = default;

	const auto& get_fence_ptr() const {
		return fence_strong;
	}

	shared_fence_t& get_fence() const {
		return *fence_strong;
	}

	auto& acquire_command_buffer() {
		const command_pool& p = pool;
		command_buffer_t buffer = p.allocate_primary_buffer();
		command_buffers.emplace_back(std::move(buffer));
		return command_buffers.back();
	}

	bool is_batch_complete() const override final {
		return (*fence_strong)->is_signaled();
	}
};

template <typename UserData>
using ste_device_queue_batch = ste_device_queue_batch_oneshot<UserData>;

}
}
