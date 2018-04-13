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
	static constexpr auto alignment = storage_policy::alignment;

	static constexpr latch_descriptor waiter_counters_offset = sizeof(latch_descriptor) * 8 / 2;

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
	static constexpr latch_descriptor mechanism_msb() { return (mask<mechanism>() + lock_bit<mechanism>()) >> 1; }

public:
	latch_descriptor load(std::memory_order mo = std::memory_order_acquire) const noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_atomic_loads;
#endif
		return latch.load(mo);
	}
	parks_counter_type load_parked(std::memory_order mo = std::memory_order_acquire) const noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_atomic_loads;
#endif
		return parks.load(mo);
	}
	
	// Attempts to acquire lock
	template <shared_futex_detail::mechanism mechanism>
	latch_descriptor acquire() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		// Attempts lock acquisition 
		const auto bits = lock_bit<mechanism>();
		return latch.fetch_add(bits);
	}
	// Re-attempts lock acquisition and decreases waiter counter
	template <shared_futex_detail::mechanism mechanism>
	latch_descriptor reattempt_acquire() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		const auto bits = lock_bit<mechanism>() - (lock_bit<mechanism>() << waiter_counters_offset);
		return latch.fetch_add(bits);
	}
	// Reverts lock acquisition and increases waiter counter
	template <shared_futex_detail::mechanism mechanism>
	latch_descriptor revert() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		const auto bits = (lock_bit<mechanism>() << waiter_counters_offset) - lock_bit<mechanism>();
		return latch.fetch_add(bits) + bits;
	}
	// Release lock
	template <shared_futex_detail::mechanism mechanism>
	latch_descriptor release() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif

		// Decrease comsumer count
		const auto bits = -lock_bit<mechanism>();
		return latch.fetch_add(bits) + bits;
	}
	
	// Counts number of active consumers
	template <shared_futex_detail::mechanism mechanism>
	static latch_descriptor consumers_in_flight(const latch_descriptor &latch_value) noexcept {
		return (latch_value & mask<mechanism>()) >> offset<mechanism>();
	}
	// Counts number of waiters
	template <shared_futex_detail::mechanism mechanism>
	static latch_descriptor waiters(const latch_descriptor &latch_value) noexcept {
		return ((latch_value >> waiter_counters_offset) & mask<mechanism>()) >> offset<mechanism>();
	}
	// Returns the count of parked thread of a given mechanism
	template <shared_futex_detail::mechanism mechanism>
	static parks_counter_type parked(const parks_counter_type &parked_value) noexcept {
		return (parked_value & mask<mechanism>()) >> offset<mechanism>();
	}

	// Generates a per-mechanism unique parking key.
	template <shared_futex_detail::mechanism mechanism>
	static latch_descriptor parking_key(const latch_descriptor &modifier) noexcept {
		// Mark the mechanism with the msb bit
		const auto x = (modifier << offset<mechanism>()) & (mechanism_msb<mechanism>() - static_cast<latch_descriptor>(1));
		return mechanism_msb<mechanism>() | x;
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
#ifdef SHARED_FUTEX_DEBUG
		assert(count);
#endif
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		parks.fetch_add(-(static_cast<latch_descriptor>(count) * lock_bit<mechanism>()));
	}
};

}
