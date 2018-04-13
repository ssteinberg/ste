// StE
// © Shlomi Steinberg, 2015-2018

#pragma once

#include <shared_futex_common.hpp>
#include <shared_futex_policies.hpp>
#include <shared_futex_latch.hpp>

#include <chrono>
#include <mutex>
#include <utility>

namespace ste {

template <typename StoragePolicy, template <typename> class Latch>
class alignas(StoragePolicy::alignment) shared_futex_t {
public:
	using storage_policy = StoragePolicy;
	using latch_type = Latch<storage_policy>;

private:
	latch_type latch;

public:
	shared_futex_t() = default;

	~shared_futex_t() {
		// Futex dtored while lock is held or pending?
		assert(latch.load() == storage_policy::initial_value);
	}

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
	explicit lock_guard(SharedFutex &futex, std::defer_lock_t) noexcept : l(&futex) {}
	explicit lock_guard(SharedFutex &futex) noexcept : l(&futex) {
		lock();
	}

	~lock_guard() noexcept {
		if (l)
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

template <typename Latch, typename BackoffPolicy, shared_futex_detail::mechanism mechanism>
class shared_futex_locking_protocol {
	using latch_descriptor = typename Latch::latch_descriptor;

	// Pre-thread random generator
	static thread_local shared_futex_detail::random_generator rand;

protected:
	shared_futex_detail::lock_status status{ shared_futex_detail::lock_status::unacquired };

protected:
	/*
	*	@brief	Checks if a latch value is valid for lock acquisition
	*/
	bool can_acquire_lock(const latch_descriptor &latch_value) const noexcept {
#ifdef SHARED_FUTEX_DEBUG
		assert(status != shared_futex_detail::lock_status::acquired);
#endif
		const auto exclusive_holders = Latch::template consumers_in_flight<shared_futex_detail::mechanism::exclusive_lock>(latch_value);
		const auto upgradeable_holders = Latch::template consumers_in_flight<shared_futex_detail::mechanism::upgradeable_lock>(latch_value);
		const auto shared_holders = Latch::template consumers_in_flight<shared_futex_detail::mechanism::shared_lock>(latch_value);

		if constexpr (mechanism == shared_futex_detail::mechanism::shared_lock ||
					  mechanism == shared_futex_detail::mechanism::upgradeable_lock) {
			// Shared lockers are permitted iff there are no exclusive holders
			return exclusive_holders == 0;
		}
		else if constexpr (mechanism == shared_futex_detail::mechanism::exclusive_lock) {
			// Exclusive lockers are permitted iff there are no holders of any kind
			return exclusive_holders == 0 && upgradeable_holders == 0 && shared_holders == 0;
		}
		else {
			static_assert(false, "wtf");
			return false;
		}
	}

	// Computes count of active exclusive waiters
	std::size_t active_exclusive_waiters_count(Latch &l, const latch_descriptor &latch_value) const noexcept {
		// We arrive after a lock attempt or lock load, which will provide the acquire memory fence.
		const auto parked_value = l.load_parked(std::memory_order_relaxed);

		// Read parked exclusive consumers
		const auto exclusive_parked = Latch::template parked<shared_futex_detail::mechanism::exclusive_lock>(parked_value);
		// Read waiters count
		const auto exclusive_waiters = Latch::template waiters<shared_futex_detail::mechanism::exclusive_lock>(latch_value);
		// Deduce active exclusive waiters
		auto exclusive_active_waiters = exclusive_waiters - exclusive_parked;

		// Make sure we don't underflow
		using S = std::make_signed_t<latch_descriptor>;
		return static_cast<std::size_t>(std::max<S>(0, static_cast<S>(exclusive_active_waiters)));
	}

	// Chooses a backoff protocol
	shared_futex_detail::backoff_aggressiveness backoff_protocol(Latch &l, const latch_descriptor &latch_value) const noexcept {
		static constexpr auto backoff_relaxed_waiters_threshold = 3;
		static constexpr auto backoff_normal_waiters_threshold = 2;
		static constexpr auto backoff_aggresive_waiters_threshold = 1;

		const auto x = active_exclusive_waiters_count(l, latch_value);
		if (x <= backoff_aggresive_waiters_threshold)
			return shared_futex_detail::backoff_aggressiveness::aggressive;
		if (x <= backoff_normal_waiters_threshold)
			return shared_futex_detail::backoff_aggressiveness::normal;
		if (x <= backoff_relaxed_waiters_threshold)
			return shared_futex_detail::backoff_aggressiveness::relaxed;
		return shared_futex_detail::backoff_aggressiveness::very_relaxed;
	}

	// Specifies backoff key
	static auto backoff_key() noexcept {
		return Latch::template parking_key<mechanism>(0);
	}

public:
	bool try_lock(Latch &l) noexcept {
		const auto prev = l.template acquire<mechanism>();
		if (!can_acquire_lock(prev)) {
			// Release lock on acquisition failure
			l.template release<mechanism>();
			return false;
		}

		status = shared_futex_detail::lock_status::acquired;
		return true;
	}

	template <typename Clock, typename Duration>
	bool try_lock_until(Latch &l, const std::chrono::time_point<Clock, Duration> &until) noexcept {
		// Attempt lock. We assume this usually succeeds.
		auto latch_value = l.template acquire<mechanism>();
		if (can_acquire_lock(latch_value)) {
			status = shared_futex_detail::lock_status::acquired;
			return true;
		}

		// Can't take lock, revert and wait.
		// We do not revert for shared waiters.
		if constexpr (mechanism != shared_futex_detail::mechanism::shared_lock)
			l.template revert<mechanism>();
		status = shared_futex_detail::lock_status::waiting;

		for (int iteration = 1;; ++iteration) {
#ifdef SHARED_FUTEX_STATS
			++shared_futex_detail::debug_statistics.iterations;
#endif

			// Once backoff policy decides to park us, register us as parked.
			const auto park_predicate = [&]() {
				return can_acquire_lock(l.load(std::memory_order_relaxed));
			};
			const auto on_park = [&]() {
#ifdef SHARED_FUTEX_DEBUG
				assert(status == shared_futex_detail::lock_status::waiting);
#endif

				l.template register_parked<mechanism>();
				status = shared_futex_detail::lock_status::parked;
				
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
			 *	Timed-out - First we check if we can acquire lock, otherwise we revert state and fail.
			 *	Unpark - Unparked by another thread, which has also unregistered us from park counters.
			 *	Park predicate was triggered - Parking failed due to park predicate, immeditately try to acquire lock.
			 *	Otherwise - Check if we can lock, and if not continue waiting.
			 */

			const bool needs_unregister_parked = result != shared_futex_detail::backoff_result::unparked &&
				status == shared_futex_detail::lock_status::parked;

			// On successful backoff we check, conservatively without ping-ponging cache lines, if we should reattempt to acquire lock.
			if (result == shared_futex_detail::backoff_result::park_predicate_triggered || 
				can_acquire_lock(l.load())) {
				if constexpr (mechanism == shared_futex_detail::mechanism::shared_lock) {
					// For shared lockers we only wait for non-shared holders to clear out
					status = shared_futex_detail::lock_status::acquired;
				}
				else {
					// For non-shared we need to actively reattempt lock acquisition
					latch_value = l.template reattempt_acquire<mechanism>();

					if (can_acquire_lock(latch_value)) {
						// We have lock
						status = shared_futex_detail::lock_status::acquired;
					}
					else {
						// Revert
						l.template revert<mechanism>();
					}
				}
			}
			
			// Unregister from parked counters if it was not done by the unparker
			if (needs_unregister_parked)
				l.template unregister_parked<mechanism>();

			// Have we acquired lock?
			if (status == shared_futex_detail::lock_status::acquired)
				return true;			
			// Timeout?
			if (result == shared_futex_detail::backoff_result::timeout) {
				status = shared_futex_detail::lock_status::unacquired;
				return false;
			}

			// Otherwise go back to waiting

			// If we have been unparked, reset iterations counter to restart backoff policy.
			if (status == shared_futex_detail::lock_status::parked)
				iteration = 0;
			status = shared_futex_detail::lock_status::waiting;
		}
	}

	void unlock(Latch &l) noexcept {
		// If exclusive waiters count is lower than this threshold, a unpark new one.
		static constexpr auto exclusive_waiters_count_thershold_for_unpark = 1;

#ifdef SHARED_FUTEX_DEBUG
		assert(status == shared_futex_detail::lock_status::acquired);
#endif

		const auto latch_value = l.template release<mechanism>();
		status = shared_futex_detail::lock_status::unacquired;

		const auto parked_value = l.load_parked(std::memory_order_acquire);

		// Try to unpark a shared waiter
		if constexpr (mechanism != shared_futex_detail::mechanism::shared_lock)
		{
			const auto shared_parked = Latch::template parked<shared_futex_detail::mechanism::shared_lock>(parked_value);
			if (shared_parked > 0) {
				const auto unpark_key = Latch::template parking_key<shared_futex_detail::mechanism::shared_lock>(0);
				const auto unparked = l.parking.unpark_all(unpark_key);

				if (unparked) {
					l.template unregister_parked<mechanism>(unparked);
#ifdef SHARED_FUTEX_STATS
					++shared_futex_detail::debug_statistics.unparks;
#endif
					return;
				}
			}
		}

		// Try to unpark an exclusive waiter
		const auto exclusive_active_waiters = active_exclusive_waiters_count(l, latch_value);
		if (exclusive_active_waiters < exclusive_waiters_count_thershold_for_unpark) {
			const auto exclusive_parked = Latch::template parked<shared_futex_detail::mechanism::exclusive_lock>(parked_value);
			if (exclusive_parked > 0) {
				// We might have a parked thread
				const auto unpark_key = Latch::template parking_key<shared_futex_detail::mechanism::exclusive_lock>(0);
				const auto unparked = l.parking.unpark_one(unpark_key);
				
				// Unregister from park counters, as needed.
				if (unparked) {
					l.template unregister_parked<mechanism>(unparked);
#ifdef SHARED_FUTEX_STATS
					++shared_futex_detail::debug_statistics.unparks;
#endif
					return;
				}
			}
		}
	}

	bool owns_lock() const noexcept { return status == shared_futex_detail::lock_status::acquired; }
};

template <typename Latch, typename BackoffPolicy, shared_futex_detail::mechanism mechanism>
thread_local shared_futex_detail::random_generator shared_futex_locking_protocol<Latch, BackoffPolicy, mechanism>::rand;

// Upgrading an upgradeable lock implementation
//
// Upgrade routines must be called on a lock that was successfully upgradeable-locked. Successfully upgrading the lock results in an exclusive hold
// on the lock, consuming the upgradeable-lock.
// Logic is different from other lockers as we have a strong guarantee: Only shared holders are possible, and no new ones are allowed. This is similar to the
// pending-exclusive situation, allowing a fast lock acquisition.
/*template <typename Lock, typename BackoffPolicy>
class lock_upgrader final : public shared_futex_detail::generic_locker<
	Lock, BackoffPolicy,
	shared_futex_detail::mechanism::upgrading_to_exclusive_lock
> {

	latch_type lock_bits() const noexcept override final {
		// We lock via reverting the upgradeable bit and setting the exclusive bit
//		if (!lock_taken)
//			return Lock::exclusive_lock_bit - Lock::upgradeable_lock_bit;
		// We unlock by reverting the exclusive bit
		return Lock::exclusive_lock_bit;
	}

	// We just need to make sure there're no more shared holders.
	bool valid(latch_type l) const noexcept override final { return (l & Lock::non_shared_mask) == (Lock::initial_value & Lock::non_shared_mask); }

	// Always try to take lock, as taking lock is simply checking the lock value.
	std::pair<bool, latch_type> should_try_lock(Lock &l, std::memory_order mo) const noexcept override final {
		return std::make_pair(true, Lock::initial_value);
	}
	bool should_try_lock_backoff_predicate(Lock &l, std::memory_order mo) const noexcept override final { return valid(l.data().latch.load(mo)); }

public:
	bool try_lock(Lock &l) noexcept override final {
//		assert(!lock_taken);

		const auto should_take = valid(l.data().latch.load());
		if (should_take) {
			// We effectively have the lock, mark it as exclusively owned
			try_lock_impl<>(l);
//			lock_taken = true;

			return true;
		}

		return false;
	}
};*/


// shared_futex helpers

using shared_futex = shared_futex_t<shared_futex_default_storage_policy, shared_futex_default_latch>;

template <typename BackoffPolicy, typename SharedFutex, typename... Args>
lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_detail::mechanism::shared_lock>> make_shared_lock(SharedFutex &l, Args &&... args) {
	return lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_detail::mechanism::shared_lock>>(l, std::forward<Args>(args)...);
}

template <typename BackoffPolicy, typename SharedFutex, typename... Args>
lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_detail::mechanism::upgradeable_lock>> make_upgradeable_lock(SharedFutex &l, Args &&... args) {
	return lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_detail::mechanism::upgradeable_lock>>(l, std::forward<Args>(args)...);
}

template <typename BackoffPolicy, typename SharedFutex, typename... Args>
lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_detail::mechanism::exclusive_lock>> make_exclusive_lock(SharedFutex &l, Args &&... args) {
	return lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_detail::mechanism::exclusive_lock>>(l, std::forward<Args>(args)...);
}

/*
*	@brief	Upgrades the lock owned by an upgradeable lock_guard to an exclusive lock, consuming the guard and returning an exclusive one.
*			Lock must have been successfully acquired via an upgrade lock.
*			
*	@param	upgradeable_guard	Must be a lock_guard using an upgradeable_locker and owning the lock
*/
template <typename BackoffPolicy, typename SharedFutex, typename B>
lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_detail::mechanism::exclusive_lock>> upgrade_lock(lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, B, shared_futex_detail::mechanism::upgradeable_lock>> &&upgradeable_guard) {
	// Upgradeable guard owns lock?
	assert(upgradeable_guard.owns_lock());

	// Read lock and dispose of the upgradeable guard
	auto lock = &upgradeable_guard.mutex();
	upgradeable_guard.release();

	// Ugrade
//	lock_upgrader<SharedFutex, BackoffPolicy>().try_lock_until(*lock, std::chrono::steady_clock::time_point::max());

	// Return an exclusive guard owning the lock
	return lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_detail::mechanism::exclusive_lock>>(*lock, std::defer_lock);
}

/*
*	@brief	Attempts to upgrade the lock owned by an upgradeable lock_guard to an exclusive lock, on success consumes the guard and
*			creates an exclusive one, otherwise leaves the upgradeable guard untouched.
*			Lock must have been successfully acquired via an upgrade lock.
*			
*	@param	upgradeable_guard	Must be a lock_guard using an upgradeable_locker and owning the lock
*
*	@return	Returns a pair of a success flag and the new exclusive guard, if successful.
*/
template <typename BackoffPolicy, typename SharedFutex, typename B, typename Clock, typename Duration>
std::pair<bool, lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_detail::mechanism::exclusive_lock>>> try_upgrade_lock_until(lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, B, shared_futex_detail::mechanism::upgradeable_lock>> &&upgradeable_guard, const std::chrono::time_point<Clock, Duration> &until) {
	// Upgradeable guard owns lock?
	assert(upgradeable_guard.owns_lock());

	// Read lock and try to upgrade
	auto lock = &upgradeable_guard.mutex();
//	if (lock_upgrader<SharedFutex, BackoffPolicy>().try_lock_until(*lock, until)) {
//		// Release old guard
//		upgradeable_guard.release();
//
//		// Return an exclusive guard owning the lock
//		return std::make_pair(true, lock_guard<SharedFutex, exclusive_locker<SharedFutex, BackoffPolicy>>(*lock, std::defer_lock));
//	}

	return std::make_pair(true, lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_detail::mechanism::exclusive_lock>>{});
}

/*
*	@brief	Attempts to upgrade the lock owned by an upgradeable lock_guard to an exclusive lock, on success consumes the guard and
*			creates an exclusive one, otherwise leaves the upgradeable guard untouched.
*			Lock must have been successfully acquired via an upgrade lock.
*			
*	@param	upgradeable_guard	Must be a lock_guard using an upgradeable_locker and owning the lock
*
*	@return	Returns a pair of a success flag and the new exclusive guard, if successful.
*/
template <typename BackoffPolicy, typename SharedFutex, typename B, typename Rep, typename Period>
std::pair<bool, lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, BackoffPolicy, shared_futex_detail::mechanism::exclusive_lock>>> try_upgrade_lock_for(lock_guard<SharedFutex, shared_futex_locking_protocol<typename SharedFutex::latch_type, B, shared_futex_detail::mechanism::upgradeable_lock>> &&upgradeable_guard, const std::chrono::duration<Rep, Period> &duration) {
	const auto until = std::chrono::steady_clock::now() + duration;
	return try_upgrade_lock_until<BackoffPolicy>(std::move(upgradeable_guard), until);
}

}
