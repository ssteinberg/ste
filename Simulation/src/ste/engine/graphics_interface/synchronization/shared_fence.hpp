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
class shared_fence : public ste_resource_pool_resetable_trait<const vk::vk_logical_device &> {
private:
	vk::vk_fence f;
	std::promise<R> promise;
	std::shared_future<R> future;

public:
	/**
	*	@brief	Construct a fence object in unsignaled state
	*/
	shared_fence(const vk::vk_logical_device &device)
		: f(device, false), future(promise.get_future().share())
	{}
	/**
	*	@brief	Construct a fence object in signaled state holding value 'val'
	*
	*	@param	val		Initial value of fence
	*/
	template <typename T, typename S = R>
	shared_fence(const vk::vk_logical_device &device, T &&val,
				 typename std::enable_if<!std::is_void<S>::value>::type* = nullptr)
		: f(device, true), future(promise.get_future().share())
	{
		promise.set_value(std::forward<T>(val));
	}
	/**
	*	@brief	Construct a fence object
	*
	*	@param	signaled	Initial state of the fence
	*/
	template <typename S = R>
	shared_fence(const vk::vk_logical_device &device, bool signaled,
				 typename std::enable_if<std::is_void<S>::value>::type* = nullptr)
		: f(device, signaled), future(promise.get_future().share())
	{
		if (signaled)
			promise.set_value();
	}
	~shared_fence() noexcept {}

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
	decltype(auto) get(typename std::enable_if<!std::is_void<S>::value>::type* = nullptr) const {
		auto& val = future.get();
		f.wait_idle();
		return val;
	}
	/**
	*	@brief	Wait for the fence to be signaled and retrieves the value/exception stored in the fence
	*/
	template <typename S = R>
	void get(typename std::enable_if<std::is_void<S>::value>::type* = nullptr) const {
		future.get();
		f.wait_idle();
	}
	/**
	*	@brief	Fence status query
	*/
	bool is_signaled() const {
		if (future.wait_for(std::chrono::nanoseconds(0)) == std::future_status::ready) {
			bool ret = f.is_signaled();
			return ret;
		}
		return false;
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
		if (future.wait_for(timeout_duration * .5f) == std::future_status::ready) {
			bool ret = f.wait_idle(timeout_duration * .5f);
			return ret;
		}
		return false;
	}

	auto& get_fence() const { return f; }
};

}
}
