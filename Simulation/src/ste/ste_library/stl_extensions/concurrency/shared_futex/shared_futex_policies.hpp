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
	// Futex alignment
	static constexpr std::size_t alignment = std::hardware_destructive_interference_size;

	/*
	 *	Locking variable bit allocation
	 */
	// Bit depth for simultaneous shared lockers
	static constexpr std::size_t shared_bits = 13;
	// Bit depth for simultaneous upgradeable lockers
	static constexpr std::size_t upgradeable_bits = 12;
	// Bit depth for simultaneous exclusive lockers
	static constexpr std::size_t exclusive_bits = 13;
	
	// Count of buckets used to distribute upgradeable parks
	static constexpr std::size_t upgradeable_park_buckets = 2;
	// Count of buckets used to distribute exclusive parks
	static constexpr std::size_t exclusive_park_buckets = 2;

	using latch_data_type = std::int64_t;
	static constexpr latch_data_type initial_value = {};

	using parks_counter_type = latch_data_type;
};


/*
 *	@brief	Spin-lock backoff policy
 */
struct spinlock_backoff_policy {
	static inline void spin(int spins = 1) noexcept {
		for (int i=0;i<spins;++i)
			::_mm_pause();
	}

	template <typename Latch, typename ParkPredicate, typename OnPark, typename ParkKey, typename Clock, typename Duration>
	static inline shared_futex_detail::backoff_return_t backoff(Latch &l,
																shared_futex_detail::backoff_aggressiveness aggressiveness,
																ParkPredicate &&park_predicate,
																OnPark &&on_park,
																int iteration,
																ParkKey &&park_key,
																const std::chrono::time_point<Clock, Duration> &until) noexcept {
		if (until != std::chrono::time_point<Clock, Duration>::max() && Clock::now() >= until)
			return { shared_futex_detail::backoff_result::timeout };

		spin(1);
		return { shared_futex_detail::backoff_result::success };
	}
};

/*
 *	@brief	Spins, yields and then parks.
 *			A spin cycle will take <10ns, a yield inccured context-switch <1000ns and a park will cost thousands ns and more in case 
 *			of contention on the parking slot. Therefore this implementation is essentially an exponential backoff policy, which is a 
 *			well studied approach to find an acceptable balance between contending processes and reduce number of collisions.
 */
struct exponential_backoff_policy {
	template <typename Latch, typename ParkPredicate, typename OnPark, typename ParkKey, typename Clock, typename Duration>
	static shared_futex_detail::backoff_return_t wait_until(Latch &l, 
															ParkPredicate &&park_predicate, 
															OnPark &&on_park,
															ParkKey &&park_key,
															const std::chrono::time_point<Clock, Duration> &until) {
		// Park
		const auto park_result = l.parking.park_until(std::forward<ParkPredicate>(park_predicate),
													  std::forward<OnPark>(on_park),
													  std::forward<ParkKey>(park_key),
													  until);

		const auto wait_state = park_result.first;
		if (wait_state == parking_lot_wait_state::signaled) {
			assert(park_result.second.has_value());
			return {
				shared_futex_detail::backoff_result::unparked,
				park_result.second.value_or(shared_futex_detail::unpark_operation::unpark)
			};
		}
		if (wait_state == parking_lot_wait_state::park_validation_failed) {
			return { shared_futex_detail::backoff_result::park_predicate_triggered };
		}
		if (wait_state == parking_lot_wait_state::timeout) {
			return { shared_futex_detail::backoff_result::timeout };
		}

		// Unreachable
		assert(false);
		return {};
	}

	template <typename Latch, typename ParkPredicate, typename OnPark, typename ParkKey, typename Clock, typename Duration>
	static inline shared_futex_detail::backoff_return_t backoff(Latch &l,
																shared_futex_detail::backoff_aggressiveness aggressiveness,
																ParkPredicate &&park_predicate,
																OnPark &&on_park,
																int iteration,
																ParkKey &&park_key,
																const std::chrono::time_point<Clock, Duration> &until) noexcept {
		static constexpr int log2_spins_on_last_iteration = 10; // 1024
		// No spinning on relaxed or very relaxed backoffs
		const int spin_iterations = 
			aggressiveness == shared_futex_detail::backoff_aggressiveness::aggressive ? 32 : 
			aggressiveness == shared_futex_detail::backoff_aggressiveness::normal ? 24 : 0;
		// Skip yielding as well on very relaxed backoffs
		const int yield_iterations = 
			aggressiveness == shared_futex_detail::backoff_aggressiveness::very_relaxed ? 0 : 10;
		// Never park on aggressive backoffs
		const bool do_not_park = aggressiveness == shared_futex_detail::backoff_aggressiveness::aggressive;

		// Spin
		if (iteration <= spin_iterations) {
			// Exponentially increasing spin count
			const auto spins = 1 << (log2_spins_on_last_iteration * iteration / spin_iterations);
			spinlock_backoff_policy::spin(spins);
		
			return { shared_futex_detail::backoff_result::success };
		}

		// Check timeout
		if (until != std::chrono::time_point<Clock, Duration>::max() && Clock::now() >= until)
			return { shared_futex_detail::backoff_result::timeout };
		
		// Yield (and never park on aggressive)
		if (iteration <= spin_iterations + yield_iterations ||
			do_not_park) {
			std::this_thread::yield();

			return { shared_futex_detail::backoff_result::success };
		}

		// Park
		return wait_until(l, 
						  std::forward<ParkPredicate>(park_predicate), 
						  std::forward<OnPark>(on_park), 
						  std::forward<ParkKey>(park_key), 
						  until);
	}
};

/*
 *	@brief	Does not spin, yields and then parks
 */
struct relaxed_backoff_policy {
	static constexpr int yield_iterations = 5;

	template <typename Latch, typename ParkPredicate, typename OnPark, typename ParkKey, typename Clock, typename Duration>
	static inline shared_futex_detail::backoff_return_t backoff(Latch &l,
																shared_futex_detail::backoff_aggressiveness aggressiveness,
																ParkPredicate &&park_predicate,
																OnPark &&on_park,
																int iteration,
																ParkKey &&park_key,
																const std::chrono::time_point<Clock, Duration> &until) noexcept {
		if (until != std::chrono::time_point<Clock, Duration>::max() && Clock::now() >= until)
			return { shared_futex_detail::backoff_result::timeout };

		if (iteration < yield_iterations && aggressiveness != shared_futex_detail::backoff_aggressiveness::very_relaxed) {
			std::this_thread::yield();
			return { shared_futex_detail::backoff_result::success };
		}

		return exponential_backoff_policy::wait_until(l,
													  std::forward<ParkPredicate>(park_predicate),
													  std::forward<OnPark>(on_park),
													  std::forward<ParkKey>(park_key),
													  until);
	}
};


/*
 *	@brief	Policy that controls a shared_futex protcol's behaviour, fairness and starvation-avoidness mechanisms
 */
struct shared_futex_protocol_policy {
	// The probability, multiplied by count of exclusive waiters, that a shared unlocker will raise the boost flag.
	static constexpr auto probability_to_raise_boost_flag_per_exclusive_waiter = .0075f;
	// The probability that an exclusive unlocker will drop the boost flag, even if there are more exlusive waiters.
	static constexpr auto probability_to_drop_boost_flag_per_parked_shared_waiter = .0075f;
	
	// The desired count of waiters using an aggressive backoff protocol, on average.
	static constexpr auto desired_aggressive_waiters_count = 1;
	// The desired count of waiters using a normal backoff protocol, on average.
	static constexpr auto desired_normal_waiters_count = 3;
	// The desired count of waiters using a relaxed backoff protocol, on average.
	static constexpr auto desired_relaxed_waiters_count = 0;

	// Each count of those iterations we re-choose the backoff protocol
	static constexpr auto refresh_backoff_protocol_every_iterations = 1;

	// When unparking shared waiters, use a reserve-and-unpark tactic if shared parked count is greater or equal to this threshold
	static constexpr auto shared_parked_count_threshold_for_reserve_and_unpark = 2;
	
	// When looking for candidates to unpark, we unpark a waiter if count of active waiters, that might block said waiter, is lower than 
	// this threshold.
	static constexpr auto active_waiters_count_thershold_for_unpark = 1;
};

}
