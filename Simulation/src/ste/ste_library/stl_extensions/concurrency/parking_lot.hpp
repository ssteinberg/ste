// StE
// © Shlomi Steinberg, 2015-2018

#pragma once

#include <condition_variable>
#include <mutex>
#include <atomic>
#include <array>
#include <type_traits>
#include <functional>
#include <optional>
#include <intrin.h>

namespace ste {

enum class parking_lot_wait_state {
	// Signaled by condition variable and returned successfully
	signaled,
	// Park validation predicate triggered
	park_validation_failed,
	// Timeout (unspecified predicate success value)
	timeout,
};

namespace parking_lot_detail {

class simple_spinner {
	std::atomic_flag f = ATOMIC_FLAG_INIT;

public:
	void lock() noexcept {
		// Spin
		if (!f.test_and_set())
			return;

		do {
			::_mm_pause();
		} while(f.test_and_set());
	}
	void unlock() noexcept {
		f.clear();
	}
};

class parking_lot_node_base {
public:
	static constexpr std::size_t key_size = 8;

private:
	using mutex_t = simple_spinner;

	mutex_t m;
	std::condition_variable_any cv;

	bool signaled{ false };

	// Node tag
	struct {
		void *id;
		std::aligned_storage_t<key_size> key;
	} tag{};

protected:
	void signal() noexcept {
		std::unique_lock<mutex_t> ul(m);

		signaled = true;
		cv.notify_one();
	}

public:
	template <typename P, typename K>
	parking_lot_node_base(P* id, K &&key) {
		using T = std::remove_cv_t<std::remove_reference_t<K>>;

		static_assert(std::is_trivially_destructible_v<T>, "key must be trivially destructible");
		static_assert(std::is_trivially_copy_constructible_v<T> || !std::is_lvalue_reference_v<K&&>,
					  "key must be trivially copy constructible when taking an l-value");
		static_assert(std::is_trivially_move_constructible_v<T> || !std::is_rvalue_reference_v<K&&>,
					  "key must be trivially move constructible when taking an r-value");
		static_assert(sizeof(T) <= key_size, "key must be no larger than key_size");

		tag.id = id;
		::new (&tag.key) T(std::forward<K>(key));
	}
	virtual ~parking_lot_node_base() noexcept = default;
	
	parking_lot_node_base(parking_lot_node_base&&) = delete;
	parking_lot_node_base(const parking_lot_node_base&) = delete;
	parking_lot_node_base &operator=(parking_lot_node_base&&) = delete;
	parking_lot_node_base &operator=(const parking_lot_node_base&) = delete;

	/*
	 *	@brief	Checks if the serialized tag equals to the supplied id and key.
	 *			K should be the same type as passed to the ctor.
	 */
	template <typename P, typename K>
	bool id_equals(P* id, const K &key) const noexcept {
		static_assert(sizeof(K) <= key_size, "key must be no larger than key_size");

		return tag.id == id && *reinterpret_cast<const K*>(&tag.key) == key;
	}
	bool is_signalled() const noexcept { return signaled; }

	// Returns a wait-performed boolean and the wait state as a pair
	template <typename ParkPredicate, typename Clock, typename Duration>
	std::pair<bool, parking_lot_wait_state> wait_until(ParkPredicate &&park_validation,
													   const std::chrono::time_point<Clock, Duration> &until) {
		const auto pred = [&]() { return signaled; };

		std::atomic_thread_fence(std::memory_order_acquire);
		if (pred())
			return { false, parking_lot_wait_state::signaled };

		{
			std::unique_lock<mutex_t> ul(m);

			if (pred())
				return { false, parking_lot_wait_state::signaled };
			// Check if we still need to park, this is needed as the signaling condition is not guarded by the local mutex, creating 
			// a race against the cv.
			if (park_validation())
				return { false, parking_lot_wait_state::park_validation_failed };

			// Park
			do {
				if (until != std::chrono::time_point<Clock, Duration>::max()) {
					// On cv timeout always return a timeout result, even if the predicate is true at that stage.
					// This allows the parker to distinguish between signaled and unsignalled cases.
					if (cv.wait_until(ul, until) == std::cv_status::timeout)
						return { true, parking_lot_wait_state::timeout };
				}
				else {
					cv.wait(ul);
				}
			} while (!pred());

			return { true, parking_lot_wait_state::signaled };
		}
	}

	// Intrusive list
	parking_lot_node_base *next{ nullptr };
	parking_lot_node_base *prev{ nullptr };
};
template <typename Data>
class parking_lot_node final : public parking_lot_node_base {
	std::optional<Data> data;

public:
	using parking_lot_node_base::parking_lot_node_base;

	/*
	*	@brief	Signals the node and constructs a Data object to be consumed by the waiter
	*/
	template <typename... Args>
	void signal(Args&&... args) noexcept {
		data.emplace(std::forward<Args>(args)...);
		parking_lot_node_base::signal();
	}
	/*
	*	@brief	Extracts the stored data object
	*/
	Data&& retrieve_data() && noexcept { return std::move(data).value(); }
};
template <>
class parking_lot_node<void> final : public parking_lot_node_base {
public:
	using parking_lot_node_base::parking_lot_node_base;
	using parking_lot_node_base::signal;

	/*
	*	@brief	Dummy
	*/
	int retrieve_data() && noexcept { return 0; }
};

class parking_lot_slot {
	static constexpr auto alignment = std::hardware_destructive_interference_size;

public:
	alignas(alignment) std::mutex m;

	// Simple intrusive dlist
	alignas(alignment) parking_lot_node_base *head{ nullptr };
	parking_lot_node_base *tail{ nullptr };

	void push_back(parking_lot_node_base *node) noexcept {
		if (tail) {
			node->prev = tail;
			tail = tail->next = node;
		}
		else {
			head = tail = node;
		}
	}

	void erase(parking_lot_node_base *node) noexcept {
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

public:
	static constexpr auto slots_count = 2048;

	/*
	*	@brief	Returns a static parking lot slot for a given id/key pair.
	*			See parking_lot_node_base::parking_lot_node_base(K&&).
	*/
	template <typename P, typename K>
	static parking_lot_slot& slot_for(P* id, const K &key) noexcept {
		const auto key_hash = std::hash<std::decay_t<K>>{}(key);
		const auto id_hash = std::hash<P*>{}(id);

		// boost::hash_combine
		const auto x = key_hash + 0x9e3779b9 + (id_hash << 6) + (id_hash >> 2);
		const auto idx = (id_hash ^ x) % slots_count;

		return slots[idx];
	}
	static std::array<parking_lot_slot, slots_count> slots;
};

}

template <typename NodeData>
class parking_lot {
	using node_t = parking_lot_detail::parking_lot_node<NodeData>;
	using park_return_t = std::conditional_t<
		!std::is_void_v<NodeData>,
		std::pair<parking_lot_wait_state, std::optional<NodeData>>,
		std::pair<parking_lot_wait_state, std::optional<int>>
	>;

private:
	template <typename ParkPredicate, typename OnPark, typename Clock, typename Duration>
	park_return_t wait(parking_lot_detail::parking_lot_slot &park,
					   node_t &node,
					   ParkPredicate &&park_predicate,
					   OnPark &&on_park,
					   const std::chrono::time_point<Clock, Duration> &until) {
		// Park
		on_park();
		auto wait_result = node.wait_until(std::forward<ParkPredicate>(park_predicate),
										   until);

		// Unregister node if wait has not been performed or timed-out,
		// Otherwise the signaling thread will do the unregistering, this avoids a deadlock on the park mutex.
		if (!wait_result.first || wait_result.second == parking_lot_wait_state::timeout) {
			std::unique_lock<std::mutex> bucket_lock(park.m);

			// Recheck signaled state under lock
			if (!node.is_signalled()) {
				park.erase(&node);
				return { wait_result.second, std::nullopt };
			}

			// Node has been signaled
			wait_result.second = parking_lot_wait_state::signaled;
		}

		// We have been signaled, extract stored data, if any.
		auto data = std::move(node).retrieve_data();

		return { wait_result.second, std::move(data) };
	}

public:
	/*
	 *  @brief	If on_park returns true, parks the calling thread in the specified key indefinitely.
	 */
	template <typename K, typename ParkPredicate, typename OnPark>
	park_return_t park(ParkPredicate &&park_predicate,
					   OnPark &&on_park,
					   K &&key) {
		return park_until(std::forward<ParkPredicate>(park_predicate),
						  std::forward<OnPark>(on_park),
						  std::forward<K>(key),
						  std::chrono::steady_clock::time_point::max());
	}
	/*
	 *	@brief	If on_park returns true, parks the calling thread in the specified slot until the timeout has expired 
	 *			or the thread was unparked. 
	*/
	template <typename K, typename ParkPredicate, typename OnPark, typename Clock, typename Duration>
	park_return_t park_until(ParkPredicate &&park_predicate,
							 OnPark &&on_park,
							 K &&key,
							 const std::chrono::time_point<Clock, Duration> &until) {
		if (park_predicate())
			return { parking_lot_wait_state::park_validation_failed, std::nullopt };

		// Create new node
		auto &park = parking_lot_detail::parking_lot_slot::slot_for(this, key);
		node_t node(this, std::forward<K>(key));

		// Register node
		{
			std::unique_lock<std::mutex> bucket_lock(park.m);
			park.push_back(&node);
		}

		// Park
		return wait(park, node,
					std::forward<ParkPredicate>(park_predicate),
					std::forward<OnPark>(on_park),
					until);
	}

	/*
	 *	@brief	Attempts to unpark a single node using args, which will be used to construct a NodeData object that
	 *			will be passed to the signaled thread.
	 *			
	 *	@return	Number of nodes that were signaled
	 */
	template <typename K, typename... Args>
	std::size_t unpark_one(const K &key, Args&&... args) {
		auto &park = parking_lot_detail::parking_lot_slot::slot_for(this, key);

		// Unpark one
		{
			std::unique_lock<std::mutex> bucket_lock(park.m);
			
			for (auto node = park.head; node; ) {
				// Signalling might destroy the node once wait_until() goes out of scope, take a copy of next.
				auto next = node->next;

				if (node->id_equals(this, key)) {
					assert(!node->is_signalled());

					park.erase(node);
					static_cast<node_t*>(node)->signal(std::forward<Args>(args)...);

					return 1;
				}

				node = next;
			}
		}

		return 0;
	}

	/*
	 *	@brief	Attempts to unpark all nodes using f, which will be used to construct a NodeData object that
	 *			will be passed to the signaled thread. f will be passed the current count of unparked nodes.
	 *			
	 *	@return	Number of nodes that were signaled
	 */
	template <typename K, typename F>
	std::size_t unpark_all_closure(const K &key, F&& f) {
		auto &park = parking_lot_detail::parking_lot_slot::slot_for(this, key);
		std::size_t count = 0;

		// Unpark all
		{
			std::unique_lock<std::mutex> bucket_lock(park.m);

			for (auto node = park.head; node; ) {
				// Signalling might destroy the node once wait_until() goes out of scope, take a copy of next.
				auto next = node->next;

				if (node->id_equals(this, key)) {
					assert(!node->is_signalled());

					// Erase from dlist, and unpark.
					park.erase(node);
					if constexpr (!std::is_void_v<NodeData>) {
						static_cast<node_t*>(node)->signal(f(count));
					}
					else {
						f(count);
						static_cast<node_t*>(node)->signal();
					}

					++count;
				}

				node = next;
			}
		}

		return count;
	}

	/*
	 *	@brief	Attempts to unpark all nodes using args, which will be used to construct a NodeData object that
	 *			will be passed to the signaled thread.
	 *			
	 *	@return	Number of nodes that were signaled
	 */
	template <typename K, typename... Args>
	std::size_t unpark_all(const K &key, const Args&... args) {
		if constexpr (!std::is_void_v<NodeData>)
			return unpark_all_closure(key, [t = NodeData(std::forward<Args>(args)...)](std::size_t) { return t; });
		else
			return unpark_all_closure(key, [](std::size_t) {});
	}

	/*
	 *	@brief	Attempts to unpark a single node using args. If a node to unpark is found, invokes f and if f returns true will unpark one node
	 *			similar to unpark_one().
	 *			
	 *	@return	Number of nodes that were signaled
	 */
	template <typename K, typename F, typename... Args>
	std::size_t try_unpark_one(const K &key, F&& f, Args&&... args) {
		auto &park = parking_lot_detail::parking_lot_slot::slot_for(this, key);

		// Unpark one
		{
			std::unique_lock<std::mutex> bucket_lock(park.m);
			
			for (auto node = park.head; node; ) {
				// Signalling might destroy the node once wait_until() goes out of scope, take a copy of next.
				auto next = node->next;

				if (node->id_equals(this, key)) {
					assert(!node->is_signalled());

					if (!f(1))
						return 0;

					park.erase(node);
					static_cast<node_t*>(node)->signal(std::forward<Args>(args)...);

					return 1;
				}

				node = next;
			}
		}

		return 0;
	}

	/*
	*	@brief	First counts how many suitable nodes can be unparked, then, if any found and while holding lock, invokes the supplied closure, 
	*			passing the count as argument. Ultimately, unparks all similarly to unpark_all().
	*			Limited to 32 nodes!
	*			
	*	@return	True if any nodes where found and unparked, false otherwise.
	*/
	template <typename K, typename F, typename... Args>
	std::size_t count_and_unpark_all(const K &key, F&& f, const Args&... args) {
		auto &park = parking_lot_detail::parking_lot_slot::slot_for(this, key);

		// Statically allocate, limiting to 32 nodes.
		std::size_t count = 0;
		std::array<node_t*, 32> nodes;

		{
			std::unique_lock<std::mutex> bucket_lock(park.m);

			// Extract suitable nodes
			for (auto node = park.head; node && count < nodes.max_size(); ) {
				if (node->id_equals(this, key)) {
					assert(!node->is_signalled());

					// Extract from dlist
					park.erase(node);
					nodes[count++] = static_cast<node_t*>(node);
				}

				node = node->next;
			}

			// Unpark
			if (count) {
				f(count);
				for (auto *n = nodes.data();n!=nodes.data()+count;++n)
					(*n)->signal(args...);

				return count;
			}
		}

		return 0;
	}

	/*
	 *	@brief	Checks atomically if the parking slot is empty
	 */
	template <typename K>
	bool is_slot_empty_hint(const K &key, std::memory_order mo = std::memory_order_relaxed) const noexcept {
		auto &park = parking_lot_detail::parking_lot_slot::slot_for(this, key);

		std::atomic_thread_fence(mo);
		return park.tail == nullptr;
	}
};

}
