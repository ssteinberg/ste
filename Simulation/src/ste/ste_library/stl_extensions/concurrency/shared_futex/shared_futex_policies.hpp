// StE
// © Shlomi Steinberg, 2015-2018

#pragma once

#include <shared_futex_common.hpp>
#include <parking_lot.hpp>

#include <intrin.h>
#include <thread>
#include <memory>

namespace ste {

/*
 *	@brief	Policy of shared_futex's data storage
 */
struct shared_futex_default_storage_policy {
	static constexpr std::size_t alignment = std::hardware_destructive_interference_size;

	// Locking variable bit allocation
	static constexpr std::size_t shared_bits = 12;
	static constexpr std::size_t upgradeable_bits = 8;
	static constexpr std::size_t exclusive_bits = 12;

	// We use the first half of the latch variable for the locking mechanism, while the other half is used for waiter counters with 
	// identical bit allocation.
	using latch_data_type = std::int64_t;
	static constexpr latch_data_type initial_value = {};

	using parks_counter_type = latch_data_type;
};


// Spin-lock backoff
struct spinlock_backoff_policy {
	static inline void spin(int spins = 1) noexcept {
		for (int i=0;i<spins;++i)
			::_mm_pause();
	}

	template <typename Latch, typename ParkPredicate, typename OnPark, typename ParkKey, typename Clock, typename Duration>
	static inline shared_futex_detail::backoff_result backoff(Latch &l,
															  shared_futex_detail::backoff_aggressiveness aggressiveness,
															  ParkPredicate &&park_predicate,
															  OnPark &&on_park,
															  int iteration,
															  ParkKey &&park_key,
															  const std::chrono::time_point<Clock, Duration> &until) noexcept {
		if (until != std::chrono::time_point<Clock, Duration>::max() && Clock::now() >= until)
			return shared_futex_detail::backoff_result::timeout;

		spin(1);
		return shared_futex_detail::backoff_result::success;
	}
};

// Spins, yields and then parks.
// A spin cycle will take <10ns, a yield inccured context-switch <1000ns and a park will cost thousands ns and more in case of contention on the parking slot.
// Therefore this implementation is essentially a kind of an exponential backoff policy, which is a well studied approach to find an acceptable balance 
// between contending processes and reduce number of collisions.
struct exponential_backoff_policy {
	template <typename Latch, typename ParkPredicate, typename OnPark, typename ParkKey, typename Clock, typename Duration>
	static shared_futex_detail::backoff_result wait_until(Latch &l, 
														  ParkPredicate &&park_predicate, 
														  OnPark &&on_park,
														  ParkKey &&park_key,
														  const std::chrono::time_point<Clock, Duration> &until) {
		// Wait
		const auto wait_state = l.parking.park_until(std::forward<ParkPredicate>(park_predicate),
													 std::forward<OnPark>(on_park),
													 std::forward<ParkKey>(park_key), 
													 until).first;

		if (wait_state == parking_lot_wait_state::signaled)
			return shared_futex_detail::backoff_result::unparked;
		if (wait_state == parking_lot_wait_state::park_validation_failed)
			return shared_futex_detail::backoff_result::park_predicate_triggered;
		if (wait_state == parking_lot_wait_state::timeout)
			return shared_futex_detail::backoff_result::timeout;

		assert(false);
		return shared_futex_detail::backoff_result::unparked;
	}

	template <typename Latch, typename ParkPredicate, typename OnPark, typename ParkKey, typename Clock, typename Duration>
	static inline shared_futex_detail::backoff_result backoff(Latch &l,
															  shared_futex_detail::backoff_aggressiveness aggressiveness,
															  ParkPredicate &&park_predicate,
															  OnPark &&on_park,
															  int iteration,
															  ParkKey &&park_key,
															  const std::chrono::time_point<Clock, Duration> &until) noexcept {
		static constexpr int log2_spins_on_last_iteration = 10; // 1024
		// No spinning on relaxed or very relaxed backoffs
		const int spin_iterations = 
			aggressiveness == shared_futex_detail::backoff_aggressiveness::aggressive ? 16 : 
			aggressiveness == shared_futex_detail::backoff_aggressiveness::normal ? 8 : 0;
		// Skip yielding as well on very relaxed backoffs
		const int yield_iterations = 
			aggressiveness == shared_futex_detail::backoff_aggressiveness::very_relaxed ? 0 : 5;

		// Spin
		if (iteration <= spin_iterations) {
#ifdef SHARED_FUTEX_STATS
			const auto start = std::chrono::high_resolution_clock::now();
#endif

			// Exponentially increasing spin count
			const auto spins = 1 << (log2_spins_on_last_iteration * iteration / spin_iterations);
			spinlock_backoff_policy::spin(spins);

#ifdef SHARED_FUTEX_STATS
			shared_futex_detail::debug_statistics.spinning_time += std::chrono::high_resolution_clock::now() - start;
#endif
			return shared_futex_detail::backoff_result::success;
		}

		// Check timeout
		if (until != std::chrono::time_point<Clock, Duration>::max() && Clock::now() >= until)
			return shared_futex_detail::backoff_result::timeout;
		
		// Yield (and never park on aggressive)
		if (iteration <= spin_iterations + yield_iterations ||
			aggressiveness == shared_futex_detail::backoff_aggressiveness::aggressive) {
			std::this_thread::yield();
			return shared_futex_detail::backoff_result::success;
		}

		// Park
		return wait_until(l, 
						  std::forward<ParkPredicate>(park_predicate), 
						  std::forward<OnPark>(on_park), 
						  std::forward<ParkKey>(park_key), 
						  until);
	}

	template <typename Latch>
	static void on_unlock(Latch &l, const typename Latch::latch_descriptor& lock_value, shared_futex_detail::mechanism mechanism) noexcept {
		// Wake up priority: Upgrading-to-exclusive and pending-exclusive waiters are given top-priority, then exclusive waiters and last are shared waiters

		/*const bool can_exclusive_lock = (lock_value & Latch::all_locks_mask) == Latch::initial_value;
		const bool can_shared_lock = (lock_value & Latch::non_shared_mask) == (Latch::initial_value & Latch::non_shared_mask);

		if (!can_exclusive_lock && !can_shared_lock)
			return;

#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_atomic_loads;
#endif
		
		const auto exclusive_waiters_count = ((lock_value >> Latch::waiters_counters_offset) & Latch::exclusive_mask) >> (Latch::storage_policy::shared_bits + Latch::storage_policy::upgradeable_bits);
		const auto exclusive_parked_count = (park_value & Latch::exclusive_mask) >> (Latch::storage_policy::shared_bits + Latch::storage_policy::upgradeable_bits);
		const auto shared_parked_count = park_value & Latch::shared_mask;

		const auto exclusive_nonparked_waiters = std::max<const typename Latch::latch_descriptor&>(0, exclusive_waiters_count - exclusive_parked_count);

		if (can_exclusive_lock && exclusive_nonparked_waiters == 0 && exclusive_parked_count > 0) {
#ifdef SHARED_FUTEX_STATS
			const auto start = std::chrono::high_resolution_clock::now();
#endif

			// Attempt to acquire lock
			static constexpr auto locking_bits = Latch::exclusive_lock_bit;
			static constexpr auto waiting_bits = Latch::exclusive_lock_bit << Latch::waiters_counters_offset;
			const auto new_lock_value = l.latch.fetch_add(locking_bits);
			if ((new_lock_value & Latch::all_locks_mask) == Latch::initial_value) {
				// Latch acquired, unpark.
				if (l.parking.unpark_one(1) == 1) {
					// Unpark successful, unregister from counters.
					l.parks.fetch_add(-Latch::exclusive_lock_bit);
					l.latch.fetch_add(-waiting_bits);
#ifdef SHARED_FUTEX_STATS
					shared_futex_detail::debug_statistics.lock_rmw_instructions += 2;
#endif
				}
				else {
					l.latch.fetch_add(-locking_bits);
#ifdef SHARED_FUTEX_STATS
					++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
				}
			}
			else {
				// Someone was faster then us
				l.latch.fetch_add(-locking_bits);
#ifdef SHARED_FUTEX_STATS
				++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
			}

#ifdef SHARED_FUTEX_STATS
			shared_futex_detail::debug_statistics.unparking_time += std::chrono::high_resolution_clock::now() - start;
			++shared_futex_detail::debug_statistics.unparks;
			++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		}

		else if (can_shared_lock && exclusive_nonparked_waiters == 0 && shared_parked_count > 0) {
#ifdef SHARED_FUTEX_STATS
			const auto start = std::chrono::high_resolution_clock::now();
#endif

			// Attempt to acquire lock
			static constexpr auto locking_bits = Latch::shared_lock_bit;
			static constexpr auto waiting_bits = Latch::shared_lock_bit << Latch::waiters_counters_offset;
			const auto new_lock_value = l.latch.fetch_add(locking_bits);
			if ((new_lock_value & Latch::non_shared_mask) == (Latch::initial_value & Latch::non_shared_mask)) {
				// Latch acquired, unregister from wait counters.
				const auto on_unpark = [&l](std::size_t unparked_count) {
					const auto c = static_cast<const typename Latch::latch_descriptor&>(unparked_count);

					// Unparking, unregister from counters.
					l.parks.fetch_add(-(c * Latch::shared_lock_bit));
					l.latch.fetch_add((c - 1) * locking_bits - c * waiting_bits);
#ifdef SHARED_FUTEX_STATS
					shared_futex_detail::debug_statistics.lock_rmw_instructions += 2;
#endif
				};
				if (!l.parking.count_and_unpark_all(0, on_unpark)) {
					// Already unparked
					l.latch.fetch_add(-locking_bits);
#ifdef SHARED_FUTEX_STATS
					++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
				}
			}
			else {
				// Someone was faster then us
				l.latch.fetch_add(-locking_bits);
#ifdef SHARED_FUTEX_STATS
				++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
			}

#ifdef SHARED_FUTEX_STATS
			shared_futex_detail::debug_statistics.unparking_time += std::chrono::high_resolution_clock::now() - start;
			++shared_futex_detail::debug_statistics.unparks;
			++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		}*/
	}
};

// Does not spin, yields and then parks
struct relaxed_backoff_policy {
	static constexpr int yield_iterations = 10;

	template <typename Latch, typename ParkPredicate, typename OnPark, typename ParkKey, typename Clock, typename Duration>
	static inline shared_futex_detail::backoff_result backoff(Latch &l,
															  shared_futex_detail::backoff_aggressiveness aggressiveness,
															  ParkPredicate &&park_predicate,
															  OnPark &&on_park,
															  int iteration,
															  ParkKey &&park_key,
															  const std::chrono::time_point<Clock, Duration> &until) noexcept {
		if (until != std::chrono::time_point<Clock, Duration>::max() && Clock::now() >= until)
			return shared_futex_detail::backoff_result::timeout;

		if (iteration < yield_iterations) {
			std::this_thread::yield();
			return shared_futex_detail::backoff_result::success;
		}

		return exponential_backoff_policy::wait_until(l,
													  std::forward<ParkPredicate>(park_predicate),
													  std::forward<OnPark>(on_park),
													  std::forward<ParkKey>(park_key),
													  until);
	}
};

}
