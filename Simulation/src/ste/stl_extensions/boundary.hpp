// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <future>
#include <utility>
#include <chrono>
#include <type_traits>

namespace StE {

template <typename T>
class boundary {
private:
	std::promise<T> promise;
	std::future<T> future;

public:
	boundary() : future(promise.get_future()) {}
	~boundary() noexcept {}

	boundary(boundary&&) = default;
	boundary &operator=(boundary&&) = default;

	/**
	*	@brief	Checks if the boundary is valid
	*/
	auto is_valid() const {
		return future.valid();
	}

	/**
	*	@brief	Signals the boundary
	*
	*	@param	val		Value to set the boundary to
	*/
	template <typename R, typename S = T>
	void signal(R &&val,
				typename std::enable_if<!std::is_void<S>::value>::type* = nullptr) {
		promise.set_value(std::forward<R>(val));
	}
	/**
	*	@brief	Signals the boundary
	*/
	template <typename S = T>
	void signal(typename std::enable_if<std::is_void<S>::value>::type* = nullptr) {
		promise.set_value();
	}

	/**
	*	@brief	Checks if the boundary is in signaled state
	*/
	bool is_signaled() const {
		return wait_for(std::chrono::nanoseconds(0));
	}

	/**
	*	@brief	Wait for the boundary to be signaled and retrieves the value stored in the boundary
	*/
	decltype(auto) get() {
		return future.get();
	}

	/**
	*	@brief	Wait for the boundary to be signaled
	*/
	void wait() const {
		future.wait();
	}
	
	/**
	*	@brief	Wait for the boundary to be signaled
	*
	*	@param	timeout_duration	Timeout
	*
	*	@return	True if boundary was signaled while or before waiting, false otherwise.
	*/
	template<class Rep, class Period>
	bool wait_for(const std::chrono::duration<Rep, Period>& timeout_duration) const {
		return future.wait_for(timeout_duration * .5f) == std::future_status::ready;
	}
};

}
