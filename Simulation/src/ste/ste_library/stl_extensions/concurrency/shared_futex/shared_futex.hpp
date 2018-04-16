// StE
// © Shlomi Steinberg, 2015-2018

#pragma once

#include <shared_futex_common.hpp>
#include <shared_futex_policies.hpp>
#include <shared_futex_latch.hpp>

#include <chrono>
#include <mutex>
#include <utility>
#include <optional>
#include <functional>

namespace ste {

template <typename StoragePolicy, template <typename> class Latch>
class shared_futex_t {
public:
	using storage_policy = StoragePolicy;
	using latch_type = Latch<storage_policy>;

private:
	latch_type latch;

public:
	shared_futex_t() = default;
	~shared_futex_t() noexcept = default;

	shared_futex_t(shared_futex_t &&) = delete;
	shared_futex_t(const shared_futex_t &) = delete;
	shared_futex_t &operator=(shared_futex_t &&) = delete;
	shared_futex_t &operator=(const shared_futex_t &) = delete;

	auto &data() noexcept { return latch; }
	const auto &data() const noexcept { return latch; }
};

template <typename SharedFutex, typename LockingProtocol>
class lock_guard {
	SharedFutex *l{ nullptr };
	LockingProtocol locker;

public:
	using mutex_type = SharedFutex;
	using locker_type = LockingProtocol;

public:
	lock_guard() = default;
	explicit lock_guard(SharedFutex &futex) noexcept : l(&futex) {
		lock();
	}
	lock_guard(SharedFutex &futex, std::defer_lock_t) noexcept : l(&futex) {}
	lock_guard(SharedFutex &futex, std::adopt_lock_t) noexcept : l(&futex), locker(std::adopt_lock) {}
	lock_guard(SharedFutex &futex, std::try_to_lock_t) noexcept : l(&futex) {
		try_lock();
	}
	template <class Rep, class Period>
	lock_guard(SharedFutex &futex, const std::chrono::duration<Rep,Period> &duration) noexcept : l(&futex) {
		try_lock_for(duration);
	}
	template <class Clock, class Duration>
	lock_guard(SharedFutex &futex, const std::chrono::time_point<Clock,Duration> &time_point) noexcept : l(&futex) {
		try_lock_until(time_point);
	}

	~lock_guard() noexcept {
		if (owns_lock())
			unlock();
	}

	lock_guard(const lock_guard &) = delete;
	lock_guard &operator=(const lock_guard &) = delete;

	lock_guard(lock_guard &&o) noexcept : l(o.l), locker(std::move(o.locker)) {
		o.l = nullptr;
	}

	lock_guard &operator=(lock_guard &&o) noexcept {
		SharedFutex{ std::move(*this) };
		l = o.l;
		o.l = nullptr;

		locker = std::move(o.locker);

		return *this;
	}

	friend void swap(lock_guard<SharedFutex, LockingProtocol> &a, lock_guard<SharedFutex, LockingProtocol> &b) noexcept {
		std::swap(a.l, b.l);
		std::swap(a.locker, b.locker);
	}
	void swap(lock_guard &o) noexcept { swap(*this, o); }

	void lock() noexcept {
		try_lock_until(std::chrono::steady_clock::time_point::max());
	}

	bool try_lock() noexcept {
		assert(l);
		return locker.try_lock(l->data());
	}

	template <typename Rep, typename Period>
	bool try_lock_for(const std::chrono::duration<Rep, Period> &duration) noexcept {
		const auto until = std::chrono::steady_clock::now() + duration;
		return try_lock_until(until);
	}

	template <typename Clock, typename Duration>
	bool try_lock_until(const std::chrono::time_point<Clock, Duration> &time_point) noexcept {
		assert(l);
		return locker.try_lock_until(l->data(), time_point);
	}

	void unlock() noexcept {
		assert(l);
		locker.unlock(l->data());
	}

	SharedFutex &mutex() noexcept { return *l; }
	const SharedFutex &mutex() const noexcept { return *l; }

	void release() noexcept {
		l = nullptr;
		locker = {};
	}

	bool owns_lock() const noexcept { return l && locker.owns_lock(); }
	operator bool() const noexcept { return owns_lock(); }
};


namespace shared_futex_detail {

struct random_generator {
	std::random_device r;
	std::mt19937 gen;
	std::uniform_real_distribution<float> dist;

	random_generator() : gen(r()), dist(0,1) {}

	auto operator()() noexcept { return dist(gen); }
};

}

template <typename Latch, typename BackoffPolicy, typename ProtocolPolicy, shared_futex_detail::modus_operandi mo>
class shared_futex_locking_protocol {
	using latch_descriptor = typename Latch::latch_descriptor;
	using parks_descriptor = typename Latch::parks_descriptor;

	// Pre-thread random generator
	static thread_local shared_futex_detail::random_generator rand;

	// Helper values
	enum class acquisition_primality{ acquirer, waiter };
	enum class unpark_tactic { one, all };
	enum class release_reason { failure, lock_release };

	using modus_operandi = shared_futex_detail::modus_operandi;
	using backoff_aggressiveness = shared_futex_detail::backoff_aggressiveness;
	using lock_status = shared_futex_detail::lock_status;
	
protected:
	lock_status status{ lock_status::unacquired };

private:
	/*
	 *	@brief	Unparks threads of a specified modus operandi and specified unparking key
	 *	@return	Count of threads successfully unparked
	 */
	template <unpark_tactic tactic, modus_operandi modus_operandi_to_unpark>
	static std::size_t unpark(Latch &l, const std::uint32_t unparking_key_user_value) noexcept {
		// Generate parking key
		const auto unpark_key = Latch::template parking_key<modus_operandi_to_unpark>(unparking_key_user_value);
		using unpark_key_t = std::decay_t<decltype(unpark_key)>;

		// Choose function for given unpark tactic
		using parking_lot_t = decltype(l.parking);
		using F = decltype(&parking_lot_t::template unpark_one<unpark_key_t>);
		F unparking_function = tactic == unpark_tactic::all ?
			&parking_lot_t::template unpark_all<unpark_key_t> :
			&parking_lot_t::template unpark_one<unpark_key_t>;

		// Attempt unpark
		const auto unparked = std::invoke(unparking_function, l.parking, unpark_key);
		if (unparked) {
			// On successful unpark it the unparker's duty to unregister from parked counters
			l.template unregister_parked<modus_operandi_to_unpark>(unparked);
#ifdef SHARED_FUTEX_STATS
			++shared_futex_detail::debug_statistics.unparks;
#endif
		}

		return unparked;
	}

	// Checks if we can allow to persitently mutate the latch during acquisition
	static bool allow_persistent_latch_mutation(const latch_descriptor &latch_value) noexcept {
		if constexpr (mo == modus_operandi::shared_lock) {
			// For shared holders we allow persistent mutation if boost flag is not set and there are no upgradeable holders.
			const auto upgradeable_consumers = latch_value.template consumers<modus_operandi::upgradeable_lock>();
			const auto upgradeable_waiters = latch_value.template waiters<modus_operandi::upgradeable_lock>();
			const auto upgradeable_holders = upgradeable_consumers - upgradeable_waiters;

			return !latch_value.boost_flag() || upgradeable_holders;
		}

		return false;
	}

protected:
	/*
	*	@brief	Checks if a latch value is valid for lock acquisition
	*/
	template <acquisition_primality primality, modus_operandi mo_to_check = mo>
	static bool can_acquire_lock(const latch_descriptor &latch_value) noexcept {
		const auto exclusive_holders = latch_value.template consumers<modus_operandi::exclusive_lock>();
		const auto upgradeable_holders = latch_value.template consumers<modus_operandi::upgradeable_lock>();
		const auto shared_holders = latch_value.template consumers<modus_operandi::shared_lock>();

		if constexpr (mo_to_check == modus_operandi::shared_lock) {
			// Shared waiters are permitted iff there are no exclusive holders,
			// while new shared lockers need to also wait for upgradeable and upgrading holders.
			if constexpr (primality == acquisition_primality::waiter)
				return exclusive_holders == 0;
			else /*(primality == acquisition_primality::acquirer)*/
				return exclusive_holders == 0 && upgradeable_holders == 0;
		}
		else if constexpr (mo_to_check == modus_operandi::upgradeable_lock) {
			// Upgradeable lockers are permitted iff there are no exclusive holders nor upgradeable holders
			return exclusive_holders == 0 && upgradeable_holders == 0;
		}
		else if constexpr (mo_to_check == modus_operandi::exclusive_lock) {
			// Exclusive lockers are permitted iff there are no holders of any kind
			return exclusive_holders == 0 && upgradeable_holders == 0 && shared_holders == 0;
		}
		else /*(mo_to_check == modus_operandi::upgrade_to_exclusive_lock)*/ {
			// We already hold an upgradeable lock, therefore we only need to wait for the shared holders to clear out.
			return shared_holders == 0;
		}
	}

	// Computes count of active waiters
	template <modus_operandi mo_to_check>
	static std::size_t active_waiters_count(const latch_descriptor &latch_value, const parks_descriptor &parked_value) noexcept {
		// Read parked consumers
		const auto parked = parked_value.template parked<mo_to_check>();
		// Read waiters count
		const auto waiters = latch_value.template waiters<mo_to_check>();
		// Deduce active waiters
		auto active_waiters = waiters - parked;

		// Make sure we don't underflow
		using S = std::make_signed_t<decltype(active_waiters)>;
		return static_cast<std::size_t>(std::max<S>(0, static_cast<S>(active_waiters)));
	}

	/*
	 *	@brief	Handles unparking of parked threads.
	 *	
	 *			Priority: 
	 *			Upgrade-to-exclusive first as they block all other lockers. Shared and upgradeable waiters come second, unless boost 
	 *			flag is set, in which case exclusive waiters come before shared and upgradeable waiters.
	 */
	static void unpark_if_needed(Latch &l, const latch_descriptor &latch_value) noexcept {
		std::optional<parks_descriptor> parked_value;

		// Try to unpark an upgrade-to-exclusive waiter (there can only be one at most)
		if constexpr (mo == modus_operandi::shared_lock) {
			if (can_acquire_lock<acquisition_primality::waiter, modus_operandi::upgrade_to_exclusive_lock>(latch_value)) {
				if (!parked_value) parked_value = l.load_parked();

				const auto upgrading_to_exclusive_parked = parked_value.value().template parked<modus_operandi::upgrade_to_exclusive_lock>();
				if (upgrading_to_exclusive_parked > 0 &&
					unpark<unpark_tactic::one, modus_operandi::upgrade_to_exclusive_lock>(l, 0))
					return;
			}
		}

		// Try to unpark all shared waiter and/or an upgradeable waiter
		if constexpr (mo != modus_operandi::shared_lock) {
			bool unparked_successfully = false;

			if (can_acquire_lock<acquisition_primality::waiter, modus_operandi::shared_lock>(latch_value)) {
				if (!parked_value) parked_value = l.load_parked();

				const auto shared_parked = parked_value.value().template parked<modus_operandi::shared_lock>();
				const auto upgradeable_parked = parked_value.value().template parked<modus_operandi::upgradeable_lock>();

				// Unpark all possible shared first
				if constexpr (mo != modus_operandi::upgradeable_lock)
					unparked_successfully = shared_parked > 0 && unpark<unpark_tactic::all, modus_operandi::shared_lock>(l, 0);
				// Unpark an upgradeable
				if (upgradeable_parked > 0) {
					const auto upgradeable_active_waiters = active_waiters_count<modus_operandi::upgradeable_lock>(latch_value, parked_value.value());
					if (upgradeable_active_waiters < ProtocolPolicy::active_waiters_count_thershold_for_unpark)
						unparked_successfully &= unpark<unpark_tactic::one, modus_operandi::upgradeable_lock>(l, 0) > 0;
				}
			}

			if (unparked_successfully)
				return;
		}

		// Try to unpark an exclusive waiter
		if (can_acquire_lock<acquisition_primality::waiter, modus_operandi::exclusive_lock>(latch_value)) {
			if (!parked_value) parked_value = l.load_parked();
			const auto exclusive_parked = parked_value.value().template parked<modus_operandi::exclusive_lock>();

			if (exclusive_parked > 0) {
				const auto exclusive_active_waiters = active_waiters_count<modus_operandi::exclusive_lock>(latch_value, parked_value.value());
				if (exclusive_active_waiters < ProtocolPolicy::active_waiters_count_thershold_for_unpark &&
					unpark<unpark_tactic::one, modus_operandi::exclusive_lock>(l, 0))
					return;
			}
		}
	}

	// Chooses a backoff protocol
	backoff_aggressiveness backoff_protocol(Latch &l, const latch_descriptor &latch_value) const noexcept {
		if constexpr (mo == modus_operandi::upgrade_to_exclusive_lock) {
			// We wait for shared holders to clear out
			// Unlike with exclusive waiters, we can't deduce anything about the remaining life-time of the shared waiters.
			return backoff_aggressiveness::normal;
		}

		// Calculate active waiters count
		const auto parked_value = l.load_parked(std::memory_order_relaxed);
		const auto x = active_waiters_count<modus_operandi::exclusive_lock>(latch_value, parked_value);
		const auto u = active_waiters_count<modus_operandi::upgradeable_lock>(latch_value, parked_value);
		// For shared lockers, we do not care about upgradeable waiters, they do not block us.
		std::size_t waiters;
		if constexpr (mo == modus_operandi::shared_lock)
			waiters = x;
		else
			waiters = x + u;
		
		if (waiters >= ProtocolPolicy::waiters_threshold_for_very_relaxed_backoff)
			return backoff_aggressiveness::very_relaxed;
		if (waiters >= ProtocolPolicy::waiters_threshold_for_relaxed_backoff)
			return backoff_aggressiveness::relaxed;
		if (waiters >= ProtocolPolicy::waiters_threshold_for_normal_backoff)
			return backoff_aggressiveness::normal;
		return backoff_aggressiveness::aggressive;
	}

	// Specifies backoff key
	static auto backoff_key() noexcept {
		return Latch::template parking_key<mo>(0);
	}

	// Attempts lock acquisition
	std::pair<bool, latch_descriptor> acquire(Latch &l) noexcept {
		// Acquire/Upgrade lock
		latch_descriptor latch_value;
		if constexpr (mo == modus_operandi::upgrade_to_exclusive_lock)
			latch_value = l.upgrade();
		else
			latch_value = l.template acquire<mo>();

		if (can_acquire_lock<acquisition_primality::acquirer>(latch_value)) {
			status = lock_status::acquired;
			return std::make_pair(true, latch_value);
		}
		return std::make_pair(false, latch_value);
	}

	// Re-attempts lock acquisition
	static auto reattempt_acquire(Latch &l) noexcept {
		if constexpr (mo == modus_operandi::upgrade_to_exclusive_lock)
			return l.upgrade();
		else
			return l.template reattempt_acquire<mo>();
	}

	// Reverts failed lock acquisition attempt
	latch_descriptor standby(Latch &l) const noexcept {
		latch_descriptor latch_value;
		if constexpr (mo == modus_operandi::upgrade_to_exclusive_lock)
			latch_value = l.revert_upgrade();
		else
			latch_value = l.template standby<mo>();
		unpark_if_needed(l, latch_value);

		return latch_value;
	}

	/* 
	 *	@brief	Releases the lock. Also called on failure (e.g. timeout).
	 *	
	 *			Priority boosting logic for exclusive lockers is performed here. Each shared unlocker might, in a stochastic manner, 
	 *			and if there're pending exclusive waiters, to trigger the boost flag, causing a priority inversion between shared and 
	 *			exclusive lockers. Likewise, exclusive unlockers have a probability to reset the boost flag, flipping the priority 
	 *			back to the default.
	 *			Those stochastic, exponential processes balance each other to avoid waiter starvation via phase-fairness.
	 */
	template <release_reason reason>
	void release(Latch &l) noexcept {
		// Release
		latch_descriptor latch_value;
		if constexpr (mo == modus_operandi::upgrade_to_exclusive_lock)
			latch_value = l.revert_upgrade();
		else
			latch_value = l.template release<mo>();

		if constexpr (reason == release_reason::lock_release) {
			const auto boost_flag = latch_value.boost_flag();

			// Shared/upgradeable holders: Periodically set the boost flag to avoid exclusive waiters starvation
			if constexpr (mo == modus_operandi::shared_lock || 
						  mo == modus_operandi::upgradeable_lock) {
				if (!boost_flag) {
					const auto shared_holders = latch_value.template consumers<modus_operandi::shared_lock>();
					const auto exclusive_waiters = latch_value.template waiters<modus_operandi::exclusive_lock>();
					if (shared_holders > 0 && exclusive_waiters > 0) {
						const auto x = rand();
						if (x < ProtocolPolicy::probability_to_raise_boost_flag_per_exclusive_waiter * exclusive_waiters)
							l.set_boost_flag();
					}
				}
			}

			// Exclusive holders: Reset boost flag if it is set and there no more exlusive waiters/holders.
			if constexpr (mo == modus_operandi::exclusive_lock ||
						  mo == modus_operandi::upgrade_to_exclusive_lock) {
				if (boost_flag) {
					const auto exclusive_holders = latch_value.template consumers<modus_operandi::exclusive_lock>();
					const auto exclusive_waiters = latch_value.template waiters<modus_operandi::exclusive_lock>();
					if (exclusive_holders == 0 && exclusive_waiters == 0)
						l.reset_boost_flag();
					else {
						// To avoid shared starvation, we also randomly drop the flag
						const auto x = rand();
						if (x < ProtocolPolicy::probability_to_drop_boost_flag_per_exclusive_unlocker)
							l.reset_boost_flag();
					}
				}
			}
		}

		unpark_if_needed(l, latch_value);
		status = lock_status::unacquired;
	}

public:
	bool try_lock(Latch &l) noexcept {
		// Attempt lock/upgrade
		if (!acquire(l).first) {
			// Release lock on acquisition failure
			release<release_reason::failure>(l);
			return false;
		}

		return true;
	}

	template <typename Clock, typename Duration>
	bool try_lock_until(Latch &l, const std::chrono::time_point<Clock, Duration> &until) noexcept {
		// Attempt lock/upgrade
		const auto acquire_result = acquire(l);
		if (acquire_result.first)
			return true;

		// Can't take lock
		auto latch_value = acquire_result.second;
		status = lock_status::waiting;

		// Choose mutation mode: Persistent or non-persistent.
		// Peristent lockers do not revert their latch mutation upon acquisition failure, e.g. shared lockers do not 
		// revert and it is only required to wait for exclusive/upgrade-to-exclusive holders to clear out.
		const bool persistent_mutation = allow_persistent_latch_mutation(latch_value);

		// Revert failed lock acquisition, if needed.
		if (!persistent_mutation)
			latch_value = standby(l);

		for (int iteration = 1;; ++iteration) {
#ifdef SHARED_FUTEX_STATS
			++shared_futex_detail::debug_statistics.iterations;
#endif

			// Once backoff policy decides to park us, register us as parked.
			const auto park_predicate = [&]() {
				return can_acquire_lock<acquisition_primality::waiter>(l.load(std::memory_order_relaxed));
			};
			const auto on_park = [&]() {
#ifdef SHARED_FUTEX_DEBUG
				assert(status == lock_status::waiting);
#endif

				l.template register_parked<mo>();
				status = lock_status::parked;
				
#ifdef SHARED_FUTEX_STATS
				++shared_futex_detail::debug_statistics.lock_parks;
#endif
			};
			// Choose backoff agressiveness protocol
			const auto aggressiveness = backoff_protocol(l, latch_value);
			// Execute back-off policy
			const auto result = BackoffPolicy::backoff(l,
													   aggressiveness,
													   park_predicate,
													   on_park,
													   iteration,
													   backoff_key(),
													   until);
			/*		Possible backoff results:
			 *	Timed-out - First we check if we can acquire lock and, if so, attempt to acquire. If we can't or fail to we revert state and 
			 *				return failure result.
			 *	Park predicate was triggered - Parking failed due to park predicate, *immeditately* try to acquire lock without checking.
			 *	Unpark - Unparked by another thread, which has also unregistered us from park counters. Reset iteration counter and continue
			 *			 as normal.
			 *	Otherwise - Normal behaviour: Pessimistically check if we can lock, and if not continue waiting.
			 */

			const bool needs_unregister_parked = result != shared_futex_detail::backoff_result::unparked &&
				status == lock_status::parked;

			// On successful backoff we check, conservatively without ping-ponging cache lines, if we should reattempt to acquire lock.
			if (result != shared_futex_detail::backoff_result::park_predicate_triggered)
				latch_value = l.load();
			if (result == shared_futex_detail::backoff_result::park_predicate_triggered || 
				can_acquire_lock<acquisition_primality::waiter>(latch_value)) {
				if (persistent_mutation) {
					// In case of persistent mutations we only need to wait for the lock conditions to be satisfied.
					status = lock_status::acquired;
				}
				else {
					// Otherwise we need to actively reattempt lock acquisition
					latch_value = reattempt_acquire(l);
					
					// Do we have lock?
					if (can_acquire_lock<acquisition_primality::waiter>(latch_value))
						status = lock_status::acquired;
					else
						// Revert
						latch_value = standby(l);
				}
			}
			
			// Unregister from parked counters if it was not done by the unparker
			if (needs_unregister_parked)
				l.template unregister_parked<mo>();

			// Have we acquired lock?
			if (status == lock_status::acquired)
				return true;			
			// Timeout?
			if (result == shared_futex_detail::backoff_result::timeout) {
				// If locker persistently mutates the latch during acquisition attempt, we revert here on failure.
				if (persistent_mutation)
					release<release_reason::failure>(l);
				status = lock_status::unacquired;
				return false;
			}

			// Otherwise go back to waiting

			// If we have been unparked, reset iterations counter to restart backoff policy.
			if (status == lock_status::parked)
				iteration = 0;
			status = lock_status::waiting;
		}
	}

	void unlock(Latch &l) noexcept {
#ifdef SHARED_FUTEX_DEBUG
		assert(status == lock_status::acquired);
#endif
		release<release_reason::lock_release>(l);
	}

	bool owns_lock() const noexcept { return status == lock_status::acquired; }

public:
	shared_futex_locking_protocol() = default;
	shared_futex_locking_protocol(std::adopt_lock_t) : status(lock_status::acquired) {}
	~shared_futex_locking_protocol() noexcept = default;

	shared_futex_locking_protocol(shared_futex_locking_protocol &&o) noexcept : status(o.status) { o.status = lock_status::unacquired; }
	shared_futex_locking_protocol &operator=(shared_futex_locking_protocol &&o) noexcept { 
		status = o.status;
		o.status = lock_status::unacquired; 

		return *this;
	}
	shared_futex_locking_protocol(const shared_futex_locking_protocol&) = delete;
	shared_futex_locking_protocol &operator=(const shared_futex_locking_protocol&) = delete;
};

template <typename Latch, typename BackoffPolicy, typename ProtocolPolicy, shared_futex_detail::modus_operandi modus_operandi>
thread_local shared_futex_detail::random_generator shared_futex_locking_protocol<Latch, BackoffPolicy, ProtocolPolicy, modus_operandi>::rand;


// shared_futex helpers

using shared_futex = shared_futex_t<shared_futex_default_storage_policy, shared_futex_default_latch>;

template <typename BackoffPolicy, typename SharedFutex, typename... Args>
lock_guard<
	SharedFutex, 
	shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_protocol_policy, shared_futex_detail::modus_operandi::shared_lock>
> 
make_shared_lock(SharedFutex &l, Args &&... args) {
	return lock_guard<
		SharedFutex, 
		shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_protocol_policy, shared_futex_detail::modus_operandi::shared_lock>
	>(l, std::forward<Args>(args)...);
}

template <typename BackoffPolicy, typename SharedFutex, typename... Args>
lock_guard<
	SharedFutex, 
	shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_protocol_policy, shared_futex_detail::modus_operandi::upgradeable_lock>
> 
make_upgradeable_lock(SharedFutex &l, Args &&... args) {
	return lock_guard<
		SharedFutex, 
		shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_protocol_policy, shared_futex_detail::modus_operandi::upgradeable_lock>
	>(l, std::forward<Args>(args)...);
}

template <typename BackoffPolicy, typename SharedFutex, typename... Args>
lock_guard<
	SharedFutex, 
	shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_protocol_policy, shared_futex_detail::modus_operandi::exclusive_lock>
> 
make_exclusive_lock(SharedFutex &l, Args &&... args) {
	return lock_guard<
		SharedFutex, 
		shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_protocol_policy, shared_futex_detail::modus_operandi::exclusive_lock>
	>(l, std::forward<Args>(args)...);
}

/*
*	@brief	Upgrades the lock owned by an upgradeable lock_guard to an exclusive lock, consuming the guard and returning an exclusive one.
*			Lock must have been successfully acquired via an upgrade lock.
*			
*	@param	upgradeable_guard	Must be a lock_guard of an upgradeable_lock modus operandi and owning the lock
*	
*	@return	An exclusive guard that owns the lock
*/
template <typename BackoffPolicy, typename SharedFutex, typename P, typename B>
auto upgrade_lock(lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, B, P, shared_futex_detail::modus_operandi::upgradeable_lock>> &&upgradeable_guard)
{
	// Upgradeable guard owns lock?
	assert(upgradeable_guard.owns_lock());

	// Upgrade
	auto& l = upgradeable_guard.mutex();
	lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, P, shared_futex_detail::modus_operandi::upgrade_to_exclusive_lock>> 
		upgrader(l, std::defer_lock);
	upgrader.lock();
	
	// Dispose of the upgradeable guard and the upgrader
	upgradeable_guard.release();
	upgrader.release();

	// Adopt the lock with a new exclusive guard
	return make_exclusive_lock(l, std::adopt_lock);
}

/*
*	@brief	Attempts to upgrade the lock owned by an upgradeable lock_guard to an exclusive lock, on success consumes the guard and
*			creates an exclusive one, otherwise leaves the upgradeable guard untouched.
*			Lock must have been successfully acquired via an upgrade lock.
*			
*	@param	upgradeable_guard	Must be a lock_guard of an upgradeable_lock modus operandi and owning the lock
*
*	@return	Returns a pair of a success flag and the new exclusive guard, if successful.
*/
template <typename BackoffPolicy, typename SharedFutex, typename P, typename B, typename Clock, typename Duration>
auto try_upgrade_lock_until(lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, B, P, shared_futex_detail::modus_operandi::upgradeable_lock>> &&upgradeable_guard, const std::chrono::time_point<Clock, Duration> &until) 
{
	using ExclusiveLockGuard = lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, P, shared_futex_detail::modus_operandi::exclusive_lock>>;

	// Upgradeable guard owns lock?
	assert(upgradeable_guard.owns_lock());

	// Upgrade
	auto& l = upgradeable_guard.mutex();
	lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, P, shared_futex_detail::modus_operandi::upgrade_to_exclusive_lock>>
		upgrader(l, std::defer_lock);
	if (!upgrader.try_lock_until(until))
		return std::make_pair(false, ExclusiveLockGuard{});

	// Dispose of the upgradeable guard and the upgrader
	upgradeable_guard.release();
	upgrader.release();
	
	// Adopt the lock with a new exclusive guard
	return std::make_pair(true, make_exclusive_lock(l, std::adopt_lock));
}

/*
*	@brief	Attempts to upgrade the lock owned by an upgradeable lock_guard to an exclusive lock, on success consumes the guard and
*			creates an exclusive one, otherwise leaves the upgradeable guard untouched.
*			Lock must have been successfully acquired via an upgrade lock.
*			
*	@param	upgradeable_guard	Must be a lock_guard of an upgradeable_lock modus operandi and owning the lock
*
*	@return	Returns a pair of a success flag and the new exclusive guard, if successful.
*/
template <typename BackoffPolicy, typename SharedFutex, typename P, typename B, typename Rep, typename Period>
auto try_upgrade_lock_for(lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, B, P, shared_futex_detail::modus_operandi::upgradeable_lock>> &&upgradeable_guard, const std::chrono::duration<Rep, Period> &duration) 
{
	const auto until = std::chrono::steady_clock::now() + duration;
	return try_upgrade_lock_until<BackoffPolicy>(std::move(upgradeable_guard), until);
}

}
