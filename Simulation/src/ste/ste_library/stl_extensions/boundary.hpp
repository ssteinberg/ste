// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <future>
#include <utility>
#include <chrono>
#include <type_traits>

namespace ste {

template <typename T>
class boundary {
private:
	std::promise<T> promise;
	std::future<T> future;

public:
	boundary() : future(promise.get_future()) {}
	~boundary() noexcept {}

	boundary(boundary &&) = default;
	boundary &operator=(boundary &&) = default;

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
				typename std::enable_if<!std::is_void<S>::value>::type * = nullptr) {
		promise.set_value(std::forward<R>(val));
	}

	/**
	*	@brief	Signals the boundary
	*/
	template <typename S = T>
	void signal(typename std::enable_if<std::is_void<S>::value>::type * = nullptr) {
		promise.set_value();
	}

	/**
	*	@brief	Signals the boundary without making the state ready immediately. The boundary will be in ready state once the calling thread is terminated.
	*
	*	@param	val		Value to set the boundary to
	*/
	template <typename R, typename S = T>
	void signal_at_thread_exit(R &&val,
							   typename std::enable_if<!std::is_void<S>::value>::type * = nullptr) {
		promise.set_value_at_thread_exit(std::forward<R>(val));
	}

	/**
	*	@brief	Signals the boundary without making the state ready immediately. The boundary will be in ready state once the calling thread is terminated.
	*/
	template <typename S = T>
	void signal_at_thread_exit(typename std::enable_if<std::is_void<S>::value>::type * = nullptr) {
		promise.set_value_at_thread_exit();
	}

	/**
	*	@brief	Signals the boundary
	*
	*	@param	e		Exception to set the boundary to
	*/
	void set_exception(const std::exception_ptr &e) {
		promise.set_exception(e);
	}

	/**
	*	@brief	Signals the boundary with exception without making the state ready immediately. The boundary will be in ready state once the calling thread is terminated.
	*
	*	@param	e		Exception to set the boundary to
	*/
	void set_exception_at_thread_exit(const std::exception_ptr &e) {
		promise.set_exception_at_thread_exit(e);
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
	template <class Rep, class Period>
	bool wait_for(const std::chrono::duration<Rep, Period> &timeout_duration) const {
		return future.wait_for(timeout_duration) == std::future_status::ready;
	}
};

}
