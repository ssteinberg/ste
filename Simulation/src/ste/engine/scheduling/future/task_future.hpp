// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "function_traits.hpp"
#include "thread_constants.hpp"

#include "future_collection.hpp"

#include <functional>
#include <memory>
#include <atomic>

#include <future>
#include <mutex>
#include <shared_mutex>

#include <chrono>

#include <type_traits>

namespace StE {

class task_scheduler;

class task_future_chaining_construct {};

/**
 *	@brief	Thread-safe wrapper around std::future. Used by StE::task_scheduler, allows future chaining as well
 *			as chaining on main thread.
 *			For thread safety task_future employs finely-grained read-write locks
 *
 *	@param R			Future return type
 *	@param is_shared	Indicates whether or not this is a shared_future
*/
template <typename R, bool is_shared>
class task_future_impl {
	friend class task_scheduler;
	template <typename, bool>
	friend class task_future_impl;

public:
	using future_type 			= typename std::conditional<is_shared, std::shared_future<R>, std::future<R>>::type;
	using chained_task_future 	= task_future_impl<R, is_shared>;
	using chaining_future_type 	= typename std::conditional<is_shared, std::shared_future<chained_task_future>, std::future<chained_task_future>>::type;

	using mutex_type		= std::shared_timed_mutex;
	using chain_mutex_type	= std::timed_mutex;
	using read_lock_type 	= std::shared_lock<mutex_type>;
	using write_lock_type 	= std::unique_lock<mutex_type>;
	using chain_lock_type 	= std::unique_lock<chain_mutex_type>;

private:
	template <typename lock_type>
	class future_lock_guard {
		lock_type ul;
		chain_lock_type cl;

	public:
		future_lock_guard(mutex_type &m, chain_mutex_type &cm, bool chain) : ul(m, std::defer_lock),
																 			 cl(cm, std::defer_lock) {
			if (chain)
				std::lock(ul, cl);
			else
				ul.lock();
		}
	};

private:
	mutable mutex_type mutex;
	mutable chain_mutex_type chain_mutex;

	task_scheduler *sched;
	future_type future;

	bool chain{ false };
	mutable chaining_future_type chaining_future;
	mutable std::shared_ptr<chained_task_future> chained_future{ nullptr };

private:
	// Helper get/wait methods. Try to lock read mutex and get or wait for future using specified timeouts.
	template <typename Future>
	auto future_get(Future &f) {
		write_lock_type rl(mutex);
		return f.get();
	}
	template <typename Future>
	auto future_wait(const Future &f) const {
		read_lock_type rl(mutex);
		return f.wait();
	}
	template <typename Future, class Rep, class Period>
	auto future_wait_for(const Future &f, std::chrono::duration<Rep,Period> timeout_duration) const {
		auto start = std::chrono::high_resolution_clock::now();

		read_lock_type rl(mutex, std::defer_lock);
		if (!rl.try_lock_for(timeout_duration))
			return std::future_status::timeout;

		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<decltype(timeout_duration)>(end - start);
		timeout_duration -= elapsed;

		return f.wait_for(timeout_duration);
	}
	template <typename Future, class Clock, class Duration>
	auto future_wait_until(const Future &f, const std::chrono::time_point<Clock,Duration>& timeout_time) const {
		read_lock_type rl(mutex, std::defer_lock);
		if (!rl.try_lock_until(timeout_time))
			return std::future_status::timeout;

		return f.wait_until(timeout_time);
	}

	// Must acquire chain_mutex before calling
	void resolve_chained_future() const {
		chained_future = std::make_shared<chained_task_future>(chaining_future.get());
	}

	auto chained_get() {
		std::atomic_thread_fence(std::memory_order_acquire);
		if (chained_future == nullptr) {
			chain_lock_type cl(chain_mutex);
			if (chained_future == nullptr) {
				resolve_chained_future();
				std::atomic_thread_fence(std::memory_order_release);
			}
		}

		return future_get(*chained_future);
	}

	void chained_wait() const {
		std::atomic_thread_fence(std::memory_order_acquire);
		if (chained_future == nullptr) {
			chain_lock_type cl(chain_mutex);
			if (chained_future == nullptr)
				resolve_chained_future();
				std::atomic_thread_fence(std::memory_order_release);
		}

		future_wait(*chained_future);
	}

	template <class Rep, class Period>
	auto chained_wait_for(std::chrono::duration<Rep,Period> timeout_duration) const {
		std::atomic_thread_fence(std::memory_order_acquire);
		if (chained_future == nullptr) {
			auto start = std::chrono::high_resolution_clock::now();

			{
				chain_lock_type cl(chain_mutex, std::defer_lock);
				if (!cl.try_lock_for(timeout_duration))
					return std::future_status::timeout;

				if (chained_future == nullptr) {
					auto wait_result = chaining_future.wait_for(timeout_duration);
					if (wait_result == std::future_status::ready) {
						resolve_chained_future();
						std::atomic_thread_fence(std::memory_order_release);
					}
					else
						return wait_result;
				}
			}

			auto end = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<decltype(timeout_duration)>(end - start);
			timeout_duration -= elapsed;
		}

		return future_wait_for(*chained_future, timeout_duration);
	}

	template <class Clock, class Duration>
	auto chained_wait_until(const std::chrono::time_point<Clock,Duration>& timeout_time) const {
		std::atomic_thread_fence(std::memory_order_acquire);
		if (chained_future == nullptr) {
			chain_lock_type cl(chain_mutex, std::defer_lock);
			if (!cl.try_lock_until(timeout_time))
				return std::future_status::timeout;

			if (chained_future == nullptr) {
				auto wait_result = chaining_future.wait_until(timeout_time);
				if (wait_result == std::future_status::ready) {
					resolve_chained_future();
					std::atomic_thread_fence(std::memory_order_release);
				}
				else
					return wait_result;
			}
		}

		return future_wait_until(*chained_future, timeout_time);
	}

	void loop_until_ready() const;

private:
	task_future_impl(typename task_future_impl<R, false>::future_type &&f, task_scheduler *sched) : sched(sched), future(std::move(f)) {}
	task_future_impl(typename task_future_impl<R, true >::future_type &&f, task_scheduler *sched) : sched(sched), future(std::move(f)) {}
	template <bool b = is_shared>
	task_future_impl(const typename task_future_impl<R, true>::future_type &f,
					 task_scheduler *sched,
					 typename std::enable_if_t<b>* = nullptr) : sched(sched), future(f) {}

	template <bool b>
	task_future_impl(write_lock_type &&wl,
					 task_future_impl<chained_task_future, b> &&f,
					 task_future_chaining_construct) : sched(f.sched),
													   chain(true),
													   chaining_future(std::move(f.future)) {
		assert(!f.chain && "Can not double chain task_futures");
	}

	task_future_impl(read_lock_type &&rl,
					 const task_future_impl<chained_task_future, true> &f,
					 task_future_chaining_construct) : sched(f.sched),
													   chain(true),
													   chaining_future(f.future) {
		assert(!f.chain && "Can not double chain task_futures");
	}

	task_future_impl(future_lock_guard<write_lock_type> &&l,
					 task_future_impl &&other) : sched(other.sched),
												 future(std::move(other.future)),
												 chain(other.chain),
												 chaining_future(std::move(other.chaining_future)),
												 chained_future(std::move(other.chained_future)) {}
	template <bool b = is_shared>
	task_future_impl(future_lock_guard<read_lock_type> &&l,
					 const task_future_impl &other,
					 std::enable_if_t<b>* = nullptr) : sched(other.sched),
													   future(other.future),
													   chain(other.chain),
													   chaining_future(other.chaining_future),
													   chained_future(other.chained_future) {}

public:
	task_future_impl() = default;

	/**
	*	@brief	Chain a future with this one. Used for then_on_main_thread().
	*
	*	@param other	Future to chain. Will be moved from.
	*/
	template <bool b>
	task_future_impl(task_future_impl<chained_task_future, b> &&other,
					 task_future_chaining_construct) : task_future_impl(write_lock_type(other.mutex),
																		std::move(other),
					 													task_future_chaining_construct()) {}
	/**
	*	@brief	Chain a future with this one. Used for then_on_main_thread().
	*
	*	@param other	Future to chain. Will be copied from.
	*/
	task_future_impl(const task_future_impl<chained_task_future, true> &other,
					 task_future_chaining_construct) : task_future_impl(read_lock_type(other.mutex),
																		other,
																		task_future_chaining_construct()) {}

	/**
	*	@brief	Move ctor.
	*
	*	@param other	Future to move.
	*/
	task_future_impl(task_future_impl &&other) : task_future_impl(future_lock_guard<write_lock_type>(other.mutex, other.chain_mutex, other.chain),
																  std::move(other)) {}
	/**
	*	@brief	Copy ctor.
	*
	*	@param other	Future to copy. Must be a shared_future.
	*/
	template <bool b = is_shared>
	task_future_impl(const task_future_impl &other,
					 std::enable_if_t<b>* = nullptr) : task_future_impl(future_lock_guard<read_lock_type>(other.mutex, other.chain_mutex, other.chain),
																		other) {}

	~task_future_impl() noexcept {}

	task_future_impl &operator=(const task_future_impl &other) = delete;

	/**
	*	@brief	Move operator.
	*
	*	@param other	Future to move.
	*/
	task_future_impl &operator=(task_future_impl &&other) {
		future_lock_guard<write_lock_type>(mutex, chain_mutex, chain);
		future_lock_guard<write_lock_type>(other.mutex, other.chain_mutex, other.chain);

		sched = other.sched;
		future = std::move(other.future);
		chain = other.chain;
		chaining_future = std::move(other.chaining_future);
		chained_future = std::move(other.chained_future);

		return *this;
	}

	/**
	*	@brief	Get future return. Will busy wait if called on main thread.
	*/
	R get() {
		if (is_main_thread())
			loop_until_ready();

		if (chain)
			return chained_get();

		return future_get(future);
	}

	/**
	*	@brief	Wait for future. Can not be called on main thread.
	*/
	void wait() const {
		assert(!is_main_thread() && "Blocking main thread");

		if (chain)
			return chained_wait();

		future_wait(future);
	}

	/**
	*	@brief	Wait for future for limited duration. Can not be called on main thread unless timeout_duration is 0.
	*
	*	@param	timeout_duration	Timeout
	*/
	template <class Rep, class Period>
	std::future_status wait_for(const std::chrono::duration<Rep,Period> &timeout_duration) const {
		assert((std::chrono::duration_cast<std::chrono::microseconds>(timeout_duration) <= std::chrono::microseconds(0) ||
				!is_main_thread()) && "Blocking main thread");

		if (chain)
			return chained_wait_for(timeout_duration);

		return future_wait_for(future, timeout_duration);
	}

	/**
	*	@brief	Wait for future until a time point. Can not be called on main thread.
	*
	*	@param	timeout_time	Timeout time point
	*/
	template <class Clock, class Duration>
	std::future_status wait_until(const std::chrono::time_point<Clock,Duration>& timeout_time) const {
		assert(!is_main_thread() && "Blocking main thread");

		if (chain)
			return chained_wait_until(timeout_time);

		return future_wait_until(future, timeout_time);
	}

	/**
	*	@brief	Schedules a lambda after this future's completion. Moves from this future and creates a new future.
	*
	*	@param	lambda	Lambda expression
	*/
	template <typename L>
	task_future_impl<typename function_traits<L>::result_t, is_shared> then(L &&lambda) &&;
	/**
	*	@brief	Schedules a lambda on the main thread after this future's completion. Moves from this future and creates a new future.
	*
	*	@param	lambda	Lambda expression
	*/
	template <typename L>
	task_future_impl<typename function_traits<L>::result_t, is_shared> then_on_main_thread(L &&lambda) &&;

	/**
	*	@brief	Moves this future into a shared future.
	*/
	task_future_impl<R, true> shared() &&;
};

template <typename R>
using task_future = task_future_impl<R, false>;
template <typename R>
using task_shared_future = task_future_impl<R, true>;

template <typename R>
using task_future_collection = future_collection<R, task_future>;

}
