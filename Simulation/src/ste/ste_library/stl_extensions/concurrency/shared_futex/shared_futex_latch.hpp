// StE
// © Shlomi Steinberg, 2015-2018

#pragma once

#include <shared_futex_common.hpp>
#include <parking_lot.hpp>

namespace ste {

/*
*	@brief	shared_futex's latch
*/
template <typename StoragePolicy>
class shared_futex_default_latch {
public:
	using storage_policy = StoragePolicy;
	using latch_descriptor = typename storage_policy::latch_data_type;
	using parks_counter_type = typename storage_policy::parks_counter_type;

	static constexpr latch_descriptor initial_value = storage_policy::initial_value;

	static constexpr latch_descriptor punch_in_offset = 0;
	static constexpr latch_descriptor punch_out_offset = sizeof(latch_descriptor) * 8 / 2;

	static constexpr auto alignment = storage_policy::alignment;

	static_assert(storage_policy::shared_bits + storage_policy::upgradeable_bits + storage_policy::exclusive_bits <= sizeof(latch_descriptor) * 8 / 2,
				  "Total bits count should take no more than half the latch size");

private:
	// Latch
	alignas(alignment) std::atomic<latch_descriptor> latch{ initial_value };

	// Parking counters
	alignas(alignment) std::atomic<parks_counter_type> parks{};

public:
	// Parking lot for smart wakeup
	parking_lot<void> parking;

private:
	// Helper methods

	template <shared_futex_detail::mechanism mechanism>
	static constexpr latch_descriptor offset() {
		switch (mechanism) {
		case shared_futex_detail::mechanism::shared_lock:
			return 0;
		case shared_futex_detail::mechanism::upgradeable_lock:
			return storage_policy::shared_bits;
		case shared_futex_detail::mechanism::exclusive_lock:
		case shared_futex_detail::mechanism::upgrading_to_exclusive_lock:
		default:
			return storage_policy::shared_bits + storage_policy::upgradeable_bits;
		}
	}
	template <shared_futex_detail::mechanism mechanism>
	static constexpr latch_descriptor bit_count() {
		switch (mechanism) {
		case shared_futex_detail::mechanism::shared_lock:
			return storage_policy::shared_bits;
		case shared_futex_detail::mechanism::upgradeable_lock:
			return storage_policy::upgradeable_bits;
		case shared_futex_detail::mechanism::exclusive_lock:
		case shared_futex_detail::mechanism::upgrading_to_exclusive_lock:
		default:
			return storage_policy::exclusive_bits;
		}
	}
	template <shared_futex_detail::mechanism mechanism>
	static constexpr latch_descriptor lock_bit() {
		return static_cast<latch_descriptor>(1) << offset<mechanism>();
	}
	template <shared_futex_detail::mechanism mechanism>
	static constexpr latch_descriptor mask() {
		static constexpr auto one = static_cast<latch_descriptor>(1);
		switch (mechanism) {
		case shared_futex_detail::mechanism::shared_lock:
			return (one << offset<shared_futex_detail::mechanism::upgradeable_lock>()) - one;
		case shared_futex_detail::mechanism::upgradeable_lock:
			static const auto mu = ~mask<shared_futex_detail::mechanism::shared_lock>();
			return ((one << offset<shared_futex_detail::mechanism::exclusive_lock>()) - one) & mu;
		case shared_futex_detail::mechanism::exclusive_lock:
		case shared_futex_detail::mechanism::upgrading_to_exclusive_lock:
		default:
			static const auto me = ~(mask<shared_futex_detail::mechanism::shared_lock>() | mask<shared_futex_detail::mechanism::upgradeable_lock>());
			static const auto x = offset<shared_futex_detail::mechanism::exclusive_lock>() + bit_count<shared_futex_detail::mechanism::exclusive_lock>();
			return ((one << x) - one) & me;
		}
	}

	static constexpr latch_descriptor all_locks_mask =
		mask<shared_futex_detail::mechanism::shared_lock>() |
		mask<shared_futex_detail::mechanism::upgradeable_lock>() |
		mask<shared_futex_detail::mechanism::exclusive_lock>();

	template <shared_futex_detail::mechanism mechanism>
	static constexpr latch_descriptor punch_counter_msb() { return (mask<mechanism>() + lock_bit<mechanism>()) >> 1; }

public:
	latch_descriptor load(std::memory_order mo = std::memory_order_acquire) const noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_atomic_loads;
#endif
		return latch.load(mo);
	}

	// Punches in and returns acquired position in queue as well as previous latch value
	template <shared_futex_detail::mechanism mechanism>
	std::pair<latch_descriptor, latch_descriptor> punch_in() noexcept {
		// Acquire a place in queue
		const auto msb = punch_counter_msb<mechanism>();
		const auto punch_in_bits = lock_bit<mechanism>() << punch_in_offset;
		const auto prev_latch_value = latch.fetch_add(punch_in_bits);

#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif

		// Read place number
		const auto position = ((prev_latch_value >> punch_in_offset) & mask<mechanism>() & ~punch_counter_msb<mechanism>()) >> offset<mechanism>();
		return std::make_pair(position, prev_latch_value);
	}
	// Punches out, returns new latch value
	template <shared_futex_detail::mechanism mechanism>
	latch_descriptor punch_out() noexcept {
		static constexpr auto msb = punch_counter_msb<mechanism>();
		const auto punch_out_bits = lock_bit<mechanism>() << punch_out_offset;
		const auto prev_latch_value = latch.fetch_add(punch_out_bits) + punch_out_bits;

#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif

		// Overflow protection
		if (((prev_latch_value >> punch_out_offset) & mask<mechanism>()) >= msb) {
			// We have used up all our bits, mask out the msb of punch in and out counters
			static constexpr auto msb_mask = ~((msb << punch_out_offset) | (msb << punch_in_offset));
			latch.fetch_and(msb_mask);
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		}

		return prev_latch_value;
	}
	// Attempts to punch in and acquire the lock immediately, on failure returns false and the latch remains intact.
	template <shared_futex_detail::mechanism mechanism>
	bool try_punch_in_and_acquire_lock() noexcept {
		const auto punch_in_bits = lock_bit<mechanism>() << punch_in_offset;

#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_atomic_loads;
#endif
		auto val = latch.load();
		decltype(val) write;
		do {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif

			if (consumers_count(val) > 0)
				return false;
			write = val + punch_in_bits;
		} while (!latch.compare_exchange_strong(val, write));

		return true;
	}

	// Acquires lock
	template <shared_futex_detail::mechanism mechanism>
	latch_descriptor acquire() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		// Increase comsumer count
		return latch.fetch_add(lock_bit<mechanism>());
	}
	// Release lock
	template <shared_futex_detail::mechanism mechanism>
	latch_descriptor release() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif

		// Decrease comsumer count
		const auto b = -lock_bit<mechanism>();
		const auto latch_value = latch.fetch_add(b) + b;

		return latch_value;
	}

	// Checks the latch current queue position
//	template <shared_futex_detail::mechanism mechanism>
//	static latch_descriptor latch_queue_position(const latch_descriptor &latch_value) noexcept {
//		return ((latch_value >> punch_out_offset) & mask<mechanism>() & ~punch_counter_msb<mechanism>()) >> offset<mechanism>();
//	}
//	// Counts number of waiters, only relevant to mechanics that use punch-in/punch-out counting.
//	template <shared_futex_detail::mechanism mechanism>
//	static latch_descriptor consumers_count(const latch_descriptor &latch_value) noexcept {
//		// Value of punch-out counter
//		const auto out_val = latch_queue_position<mechanism>(latch_value);
//		// Value of punch-in counter
//		const auto in_val = ((latch_value >> punch_in_offset) & mask<mechanism>() & ~punch_counter_msb<mechanism>()) >> offset<mechanism>();
//
//		static constexpr auto m = ((punch_counter_msb<mechanism>() >> offset<mechanism>()) - static_cast<latch_descriptor>(1));
//		return (in_val - out_val) & m;
//	}

	// Counts number of consumers, only relevant to mechanics that use acquire/release locking.
	template <shared_futex_detail::mechanism mechanism>
	static latch_descriptor consumers_in_flight(const latch_descriptor &latch_value) noexcept {
		return (latch_value & mask<mechanism>()) >> offset<mechanism>();
	}

	// Generates a per-mechanism and per-queue-position unique parking key.
	template <shared_futex_detail::mechanism mechanism>
	static latch_descriptor parking_key(const latch_descriptor &modifier) noexcept {
		// Mark the mechanism with the msb bit
		const auto x = (modifier << offset<mechanism>()) & (punch_counter_msb<mechanism>() - static_cast<latch_descriptor>(1));
		return punch_counter_msb<mechanism>() | x;
	}

	// Returns the count of parked thread of a given mechanism
	template <shared_futex_detail::mechanism mechanism>
	parks_counter_type load_parked_count(std::memory_order mo = std::memory_order_acquire) const noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_atomic_loads;
#endif

		const auto parked_value = parks.load(mo);
		return (parked_value & mask<mechanism>()) >> offset<mechanism>();
	}
	// Register parked thread
	template <shared_futex_detail::mechanism mechanism>
	void register_parked() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif

		parks.fetch_add(lock_bit<mechanism>());
	}
	// Unregister parked thread(s)
	template <shared_futex_detail::mechanism mechanism>
	void unregister_parked(const std::size_t count = 1) noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif

		parks.fetch_add(-(static_cast<latch_descriptor>(count) * lock_bit<mechanism>()));
	}
};

}
