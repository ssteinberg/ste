// StE
// © Shlomi Steinberg, 2015-2018

#pragma once

#include <condition_variable>
#include <mutex>
#include <atomic>

namespace ste {

namespace parking_lot_detail {

enum class parking_lot_wait_state {
	signalled,
	// Signalled and returned successfully
	unsignalled,
	// Unsignalled and returned successfully
	timeout,
	// Timeout (unspecified predicate success value)
};

class parking_lot_node {
	bool signaled{ false };
	std::mutex m;
	std::condition_variable cv;

public:
	void signal() {
		std::unique_lock<std::mutex> ul(m);

		signaled = true;
		// Synchronizes with the acquire fences in wait_until()
		std::atomic_thread_fence(std::memory_order_release);

		cv.notify_one();
	}

	bool is_signalled() const noexcept { return signaled; }

	template <typename F, typename Clock, typename Duration>
	parking_lot_wait_state wait_until(F &&pre_lock_predicate, const std::chrono::time_point<Clock, Duration> &until) {
		const auto pred = [&]() {
			std::atomic_thread_fence(std::memory_order_acquire);
			return signaled;
		};

		if (pred())
			return parking_lot_wait_state::unsignalled;

		{
			std::unique_lock<std::mutex> ul(m);

			// Check if we still need to park, this is needed as the signalling condition is not guarded by the local mutex, creating a race against the cv.
			if (pre_lock_predicate())
				return parking_lot_wait_state::unsignalled;

			// Wait and pool predicate
			while (!pred()) {
				if (until != std::chrono::time_point<Clock, Duration>::max()) {
					// On cv timeout always return a timeout result, even if the predicate is true at that stage.
					// This allows the parker to distinguish between signalled and unsignalled cases.
					if (cv.wait_until(ul, until) == std::cv_status::timeout)
						return parking_lot_wait_state::timeout;
				}
				else {
					cv.wait(ul);
				}
			}

			return parking_lot_wait_state::signalled;
		}
	}

	// Intrusive list
	parking_lot_node *next{ nullptr };
	parking_lot_node *prev{ nullptr };
};

struct parking_lot_slot {
	std::mutex m;

	// Simple intrusive dlist
	parking_lot_node *head{ nullptr };
	parking_lot_node *tail{ nullptr };

	void push_front(parking_lot_node *node) noexcept {
		if (head) {
			node->next = head;
			head = head->prev = node;
		}
		else {
			head = tail = node;
		}
	}

	void push_back(parking_lot_node *node) noexcept {
		if (tail) {
			node->prev = tail;
			tail = tail->next = node;
		}
		else {
			head = tail = node;
		}
	}

	parking_lot_node *pop_front() noexcept {
		auto node = head;

		if (tail != head) {
			head = head->next;
			head->prev = nullptr;
		}
		else
			head = tail = nullptr;

		return node;
	}

	void erase(parking_lot_node *node) noexcept {
		if (node->next)
			node->next->prev = node->prev;
		else
			tail = node->prev;
		if (node->prev)
			node->prev->next = node->next;
		else
			head = node->next;
	}

	void clear() noexcept { head = tail = nullptr; }
	bool is_empty() const noexcept { return head == nullptr; }
};

}

enum class parking_lot_park_priority {
	low,
	high
};

template <std::size_t Slots, std::size_t alignment = std::hardware_destructive_interference_size>
class parking_lot {
	using slot_t = parking_lot_detail::parking_lot_slot;
	alignas(alignment)std::array<slot_t, Slots> parks;

public:
	/*
	 *  @brief	If pre_lock_predicate returns true, parks the calling thread in the specified slot indefinitely.
	 */
	template <typename F>
	void park(F &&pre_lock_predicate, std::size_t slot, parking_lot_park_priority priority = parking_lot_park_priority::low) {
		park_until(std::forward<F>(pre_lock_predicate), slot, std::chrono::steady_clock::time_point::max(), priority);
	}

	/*
	*  @brief	If pre_lock_predicate returns true, parks the calling thread in the specified slot until the timeout has expired 
	*			or the thread was unparked. 
	*/
	template <typename F, typename Clock, typename Duration>
	bool park_until(F &&pre_lock_predicate, std::size_t slot, const std::chrono::time_point<Clock, Duration> &until, parking_lot_park_priority priority = parking_lot_park_priority::low) {
		auto &park = parks[slot];
		// Create new node
		parking_lot_detail::parking_lot_node node;

		// Register node
		{
			std::unique_lock<std::mutex> bucket_lock(park.m);
			if (priority == parking_lot_park_priority::high)
				park.push_front(&node);
			else
				park.push_back(&node);
		}

		// Wait
		auto wait_result = node.wait_until(std::forward<F>(pre_lock_predicate), until);

		// Unregister node if the wait was interrupt by any mechanic except signalling,
		// otherwise the signaling thread will do the unregistering, this avoids a deadlock on the park mutex.
		if (wait_result != parking_lot_detail::parking_lot_wait_state::signalled) {
			std::unique_lock<std::mutex> bucket_lock(park.m);

			// Check signalled state under lock
			if (node.is_signalled())
				return true;

			park.erase(&node);
		}

		return wait_result != parking_lot_detail::parking_lot_wait_state::timeout;
	}

	/*
	 *	@brief	Attempts to unpark one waiter.
	 *			High priority parks will unpark ahead of low priority ones.
	 *			
	 *	@return	True on success, false if no waiters were registered.
	 */
	bool unpark_one(std::size_t slot) {
		auto &park = parks[slot];

		// Unpark one
		{
			std::unique_lock<std::mutex> bucket_lock(park.m);
			if (auto *node = park.pop_front()) {
				assert(!node->is_signalled());
				node->signal();

				return true;
			}
		}

		return false;
	}

	/*
	 *	@brief	Attempts to unpark all waiters.
	 *			High priority parks will unpark ahead of low priority ones.
	 *			
	 *	@return	True on success, false if no waiters were registered.
	 */
	bool unpark_all(std::size_t slot) {
		auto &park = parks[slot];
		bool return_value;

		// Unpark all
		{
			std::unique_lock<std::mutex> bucket_lock(park.m);

			auto node = park.head;
			return_value = !!node;

			while (node) {
				// Signalling might destroy the node once wait_until() goes out of scope, take a copy of next.
				auto next = node != park.tail ? node->next : nullptr;

				assert(!node->is_signalled());
				node->signal();

				node = next;
			}
			park.clear();
		}

		return return_value;
	}
};

}
