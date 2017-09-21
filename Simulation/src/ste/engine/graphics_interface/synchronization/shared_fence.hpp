//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_logical_device.hpp>
#include <vk_fence.hpp>

#include <ste_resource_pool_traits.hpp>

#include <future>
#include <chrono>
#include <functional>
#include <type_traits>

namespace ste {
namespace gl {

template <typename R>
class shared_fence : public ste_resource_pool_resetable_trait<const vk::vk_logical_device<> &, const char*> {
private:
	vk::vk_fence<> f;
	std::promise<R> promise;
	std::shared_future<R> future;

private:
	bool is_future_signalled() const {
		return future.wait_for(std::chrono::nanoseconds(0)) == std::future_status::ready;
	}

public:
	/**
	*	@brief	Construct a fence object in unsignaled state
	*/
	shared_fence(const vk::vk_logical_device<> &device,
				 const char *name)
		: f(device, 
			name,
			false), 
		future(promise.get_future().share())
	{}
	/**
	*	@brief	Construct a fence object in signaled state holding value 'val'
	*
	*	@param	val		Initial value of fence
	*/
	template <typename T, typename S = R>
	shared_fence(const vk::vk_logical_device<> &device,
				 T &&val,
				 const char *name,
				 typename std::enable_if<!std::is_void<S>::value>::type* = nullptr)
		: f(device,
			name,
			true), 
		future(promise.get_future().share())
	{
		promise.set_value(std::forward<T>(val));
	}
	/**
	*	@brief	Construct a fence object
	*
	*	@param	signaled	Initial state of the fence
	*/
	template <typename S = R>
	shared_fence(const vk::vk_logical_device<> &device, 
				 bool signaled,
				 const char *name,
				 typename std::enable_if<std::is_void<S>::value>::type* = nullptr)
		: f(device, 
			name,
			signaled), 
		future(promise.get_future().share())
	{
		if (signaled)
			promise.set_value();
	}
	~shared_fence() noexcept {
		if (is_valid()) {
			// Destroying while the vk_fence might still be in use will cause race conditions
			assert(is_future_signalled());
			future.wait();
		}
	}

	shared_fence(shared_fence&&) = default;
	shared_fence &operator=(shared_fence&&) = default;

	/**
	*	@brief	Checks if the fence future is valid
	*/
	auto is_valid() const {
		return future.valid();
	}

	/**
	*	@brief	Resets fence to unsignaled state
	*			Not thread-safe.
	*/
	void reset() override {
		f.reset();
		promise = std::promise<R>();
		future = promise.get_future().share();
	}

	/**
	*	@brief	Signals the fence
	*
	*	@param	val		Value to set the fence to
	*/
	template <typename T, typename S = R>
	void signal(T &&val,
				typename std::enable_if<!std::is_void<S>::value>::type* = nullptr) {
		promise.set_value(std::forward<T>(val));
	}
	/**
	*	@brief	Signals the fence
	*/
	template <typename S = R>
	void signal(typename std::enable_if<std::is_void<S>::value>::type* = nullptr) {
		promise.set_value();
	}

	/**
	*	@brief	Signals the fence
	*
	*	@param	e		Exception to set the fence to
	*/
	void set_exception(const std::exception_ptr &e) {
		promise.set_exception(e);
	}

	/**
	*	@brief	Wait for the fence to be signaled and retrieves the value/exception stored in the fence
	*/
	template <typename S = R>
	decltype(auto) get_wait(typename std::enable_if<!std::is_void<S>::value>::type* = nullptr) const {
		auto& val = future.get();
		f.wait_idle();
		return val;
	}
	/**
	*	@brief	Wait for the fence to be signaled and retrieves the value/exception stored in the fence
	*/
	template <typename S = R>
	void get_wait(typename std::enable_if<std::is_void<S>::value>::type* = nullptr) const {
		future.get();
		f.wait_idle();
	}
	/**
	*	@brief	Fence status query
	*/
	bool is_signaled() const {
		return is_future_signalled() && f.is_signaled();
	}
	/**
	*	@brief	Wait for the fence to be signaled
	*/
	void wait_idle() const {
		future.wait();
		f.wait_idle();
	}
	/**
	*	@brief	Wait for the fence to be signaled
	*
	*	@param	timeout_duration	Timeout
	*
	*	@return	True if fence was signaled while or before waiting, false otherwise.
	*/
	template<class Rep, class Period>
	bool wait_for(const std::chrono::duration<Rep, Period>& timeout_duration) const {
		const auto start = std::chrono::high_resolution_clock::now();
		if (future.wait_for(timeout_duration) == std::future_status::ready) {
			const auto end = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<decltype(timeout_duration)>(end - start);

			auto timeout_duration_left = timeout_duration - elapsed;
			return f.wait_idle(timeout_duration_left);
		}
		return false;
	}

	auto& get_fence() const { return f; }
};

}
}
