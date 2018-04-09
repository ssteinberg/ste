// StE
// © Shlomi Steinberg, 2015-2018

#pragma once

#include <shared_futex_common.hpp>
#include <parking_lot.hpp>

#include <intrin.h>
#include <thread>
#include <memory>

namespace ste {

struct shared_futex_default_storage_policy {
	static constexpr std::size_t alignment = std::hardware_destructive_interference_size;

	// Locking variable bit allocation
	static constexpr std::size_t shared_bits = 12;
	static constexpr std::size_t upgradeable_bits = 8;
	static constexpr std::size_t exclusive_bits = 12;

	// We use the first half of the latch variable for the locking mechanism, while the other half is used for waiter counters with 
	// identical bit allocation.
	using latch_type = std::int64_t;
	static constexpr latch_type initial_value = 0;

	struct storage_type {
		// Lock
		alignas(alignment) std::atomic<latch_type> latch{ initial_value };
		// Parking counters
		alignas(alignment) std::atomic<latch_type> parks{ 0 };

		// Parking lot for smart wakeup 
		parking_lot<void> parking;
	};
};

// Spin-lock backoff
struct spinlock_backoff_policy {
	static inline void spin(int spins = 1) noexcept {
		for (int i=0;i<spins;++i)
			::_mm_pause();
	}

	template <typename Lock, typename ParkPredicate, typename OnPark, typename Clock, typename Duration>
	inline shared_futex_detail::backoff_result backoff(Lock &l, typename Lock::latch_type lock_value, ParkPredicate &&, OnPark &&, shared_futex_detail::mechanism, int iteration, const std::chrono::time_point<Clock, Duration> &until) const noexcept {
		if (until != std::chrono::time_point<Clock, Duration>::max() && Clock::now() >= until)
			return shared_futex_detail::backoff_result::timeout;

		spin(iteration);
		return shared_futex_detail::backoff_result::success;
	}

	template <typename Lock>
	void on_unlock(Lock &l, typename Lock::latch_type lock_value, shared_futex_detail::mechanism) const noexcept {}
};

// Spins, yields and then parks.
// A spin cycle will take <10ns, a yield inccured context-switch <1000ns and a park will cost thousands ns and more in case of contention on the parking slot.
// Therefore this implementation is essentially a kind of an exponential backoff policy, which is a well studied approach to find an acceptable balance 
// between contending processes and reduce number of collisions.
// This backoff policy expects to work on shared_futexes that use a shared_futex_default_storage_policy-like policy which provides a parking lot for parking
// waiters and the ability to cheaply register waiters.
struct exponential_backoff_policy {
	static constexpr int spin_iterations = 8;
	static constexpr int log2_spins_on_last_iteration = 10; // 1024
	static constexpr int yield_iterations = 5;

	template <typename Lock, typename ParkPredicate, typename OnPark, typename Clock, typename Duration>
	static shared_futex_detail::backoff_result wait_until(Lock &l, 
														  ParkPredicate &&park_predicate, 
														  OnPark &&on_park, 
														  shared_futex_detail::mechanism mechanism, 
														  const std::chrono::time_point<Clock, Duration> &until) {
		// Wait
		parking_lot_wait_state wait_state;
		switch (mechanism) {
		case shared_futex_detail::mechanism::shared_lock:
		case shared_futex_detail::mechanism::upgradeable_lock:
			wait_state = l.data().parking.park_until(std::forward<ParkPredicate>(park_predicate),
													 std::forward<OnPark>(on_park),
													 0, until).first;
			break;
		default:
			wait_state = l.data().parking.park_until(std::forward<ParkPredicate>(park_predicate),
													 std::forward<OnPark>(on_park),
													 1, until).first;
			break;
		}

		if (wait_state == parking_lot_wait_state::signaled)
			return shared_futex_detail::backoff_result::unparked;
		if (wait_state == parking_lot_wait_state::park_validation_failed)
			return shared_futex_detail::backoff_result::park_predicate_triggered;
		if (wait_state == parking_lot_wait_state::timeout)
			return shared_futex_detail::backoff_result::timeout;

		assert(false);
		return shared_futex_detail::backoff_result::unparked;
	}

	template <typename Lock, typename ParkPredicate, typename OnPark, typename Clock, typename Duration>
	inline shared_futex_detail::backoff_result backoff(Lock &l,
													   typename Lock::latch_type lock_value, 
													   ParkPredicate &&park_predicate,
													   OnPark &&on_park, 
													   shared_futex_detail::mechanism mechanism, 
													   int iteration, 
													   const std::chrono::time_point<Clock, Duration> &until) const noexcept {
		const auto exclusive_waiters_threshold_to_skip_spin = 2;
		const auto exclusive_waiters_threshold_to_skip_yield = 3;

		// If we have enough exclusive waiters then skip the spinning/yielding to avoid wasting resources.
		const auto park_value = l.data().parks.load();
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_atomic_loads;
#endif

		const auto exclusive_waiters_count = ((lock_value >> Lock::waiters_counters_offset) & Lock::exclusive_mask) >> (Lock::storage_policy::shared_bits + Lock::storage_policy::upgradeable_bits);
		const auto exclusive_parked_count = (park_value & Lock::exclusive_mask) >> (Lock::storage_policy::shared_bits + Lock::storage_policy::upgradeable_bits);
		auto exclusive_nonparked_waiters = exclusive_waiters_count - exclusive_parked_count;
		if (mechanism == shared_futex_detail::mechanism::exclusive_lock)	// Do not count ourselves
			--exclusive_nonparked_waiters;

		if (exclusive_nonparked_waiters < exclusive_waiters_threshold_to_skip_spin &&
			iteration <= spin_iterations) {
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

		if (exclusive_nonparked_waiters < exclusive_waiters_threshold_to_skip_yield &&
			iteration <= spin_iterations + yield_iterations) {
			// Yield
			std::this_thread::yield();
			return shared_futex_detail::backoff_result::success;
		}
	
		// Park
		return wait_until(l, std::forward<ParkPredicate>(park_predicate), std::forward<OnPark>(on_park), mechanism, until);
	}

	template <typename Lock>
	void on_unlock(Lock &l, typename Lock::latch_type lock_value, shared_futex_detail::mechanism mechanism) const noexcept {
		// Wake up priority: Upgrading-to-exclusive and pending-exclusive waiters are given top-priority, then exclusive waiters and last are shared waiters

		const bool can_exclusive_lock = (lock_value & Lock::all_locks_mask) == Lock::initial_value;
		const bool can_shared_lock = (lock_value & Lock::non_shared_mask) == (Lock::initial_value & Lock::non_shared_mask);

		if (!can_exclusive_lock && !can_shared_lock)
			return;

		const auto park_value = l.data().parks.load();
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_atomic_loads;
#endif
		
		const auto exclusive_waiters_count = ((lock_value >> Lock::waiters_counters_offset) & Lock::exclusive_mask) >> (Lock::storage_policy::shared_bits + Lock::storage_policy::upgradeable_bits);
		const auto exclusive_parked_count = (park_value & Lock::exclusive_mask) >> (Lock::storage_policy::shared_bits + Lock::storage_policy::upgradeable_bits);
		const auto shared_parked_count = park_value & Lock::shared_mask;

		const auto exclusive_nonparked_waiters = std::max<typename Lock::latch_type>(0, exclusive_waiters_count - exclusive_parked_count);

		if (can_exclusive_lock && exclusive_nonparked_waiters == 0 && exclusive_parked_count > 0) {
#ifdef SHARED_FUTEX_STATS
			const auto start = std::chrono::high_resolution_clock::now();
#endif

			// Attempt to acquire lock
			static constexpr auto locking_bits = Lock::exclusive_lock_bit;
			static constexpr auto waiting_bits = Lock::exclusive_lock_bit << Lock::waiters_counters_offset;
			const auto new_lock_value = l.data().latch.fetch_add(locking_bits);
			if ((new_lock_value & Lock::all_locks_mask) == Lock::initial_value) {
				// Lock acquired, unpark.
				if (l.data().parking.unpark_one(1) == 1) {
					// Unpark successful, unregister from counters.
					l.data().parks.fetch_add(-Lock::exclusive_lock_bit);
					l.data().latch.fetch_add(-waiting_bits);
#ifdef SHARED_FUTEX_STATS
					shared_futex_detail::debug_statistics.lock_rmw_instructions += 2;
#endif
				}
				else {
					l.data().latch.fetch_add(-locking_bits);
#ifdef SHARED_FUTEX_STATS
					++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
				}
			}
			else {
				// Someone was faster then us
				l.data().latch.fetch_add(-locking_bits);
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
			static constexpr auto locking_bits = Lock::shared_lock_bit;
			static constexpr auto waiting_bits = Lock::shared_lock_bit << Lock::waiters_counters_offset;
			const auto new_lock_value = l.data().latch.fetch_add(locking_bits);
			if ((new_lock_value & Lock::non_shared_mask) == (Lock::initial_value & Lock::non_shared_mask)) {
				// Lock acquired, unregister from wait counters.
				const auto on_unpark = [&l](std::size_t unparked_count) {
					const auto c = static_cast<typename Lock::latch_type>(unparked_count);

					// Unparking, unregister from counters.
					l.data().parks.fetch_add(-(c * Lock::shared_lock_bit));
					l.data().latch.fetch_add((c - 1) * locking_bits - c * waiting_bits);
#ifdef SHARED_FUTEX_STATS
					shared_futex_detail::debug_statistics.lock_rmw_instructions += 2;
#endif
				};
				if (!l.data().parking.count_and_unpark_all(0, on_unpark)) {
					// Already unparked
					l.data().latch.fetch_add(-locking_bits);
#ifdef SHARED_FUTEX_STATS
					++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
				}
			}
			else {
				// Someone was faster then us
				l.data().latch.fetch_add(-locking_bits);
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
	}
};

// Does not spin, yields and then parks
struct relaxed_backoff_policy {
	static constexpr int yield_iterations = 10;

	template <typename Lock, typename ParkPredicate, typename OnPark, typename Clock, typename Duration>
	inline shared_futex_detail::backoff_result backoff(Lock &l, typename Lock::latch_type, ParkPredicate &&park_predicate, OnPark &&on_park, shared_futex_detail::mechanism mechanism, int iteration, const std::chrono::time_point<Clock, Duration> &until) const noexcept {
		if (until != std::chrono::time_point<Clock, Duration>::max() && Clock::now() >= until)
			return shared_futex_detail::backoff_result::timeout;

		if (iteration < yield_iterations) {
			std::this_thread::yield();
			return shared_futex_detail::backoff_result::success;
		}
		else
			return exponential_backoff_policy::wait_until(l, 
														  std::forward<ParkPredicate>(park_predicate), 
														  std::forward<OnPark>(on_park), 
														  mechanism, until);
	}

	template <typename Lock>
	void on_unlock(Lock &l, typename Lock::latch_type lock_value, shared_futex_detail::mechanism mechanism) const noexcept {
		exponential_backoff_policy::on_unlock(l, lock_value, mechanism);
	}
};

}
