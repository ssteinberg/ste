// StE
// © Shlomi Steinberg, 2015-2018

#pragma once

#include <shared_futex_common.hpp>
#include <shared_futex_policies.hpp>

#include <chrono>
#include <mutex>
#include <utility>

namespace ste {

template <typename StoragePolicy>
class alignas(StoragePolicy::alignment) shared_futex_t {
public:
	using storage_policy = StoragePolicy;
	using storage_type = typename storage_policy::storage_type;
	using atomic_latch_type = decltype(std::declval<storage_type>().latch);
	using latch_type = typename atomic_latch_type::value_type;

	static constexpr latch_type initial_value = storage_policy::initial_value;

	static constexpr latch_type shared_lock_bit = static_cast<latch_type>(1);
	static constexpr latch_type upgradeable_lock_bit = static_cast<latch_type>(1) << storage_policy::shared_bits;
	static constexpr latch_type exclusive_lock_bit = static_cast<latch_type>(1) << (storage_policy::shared_bits + storage_policy::upgradeable_bits);

	static constexpr latch_type shared_mask = upgradeable_lock_bit - static_cast<latch_type>(1);
	static constexpr latch_type upgradeable_mask = (exclusive_lock_bit - static_cast<latch_type>(1)) & ~shared_mask;
	static constexpr latch_type exclusive_mask = ((exclusive_lock_bit << storage_policy::exclusive_bits) - static_cast<latch_type>(1)) & ~(upgradeable_mask | shared_mask);
	static constexpr latch_type non_shared_mask = upgradeable_mask | exclusive_mask;
	static constexpr latch_type all_locks_mask = shared_mask | upgradeable_mask | exclusive_mask;

	static constexpr latch_type waiters_counters_offset = storage_policy::shared_bits + storage_policy::upgradeable_bits + storage_policy::exclusive_bits;

private:
	storage_type storage;

public:
	shared_futex_t() = default;

	~shared_futex_t() {
		// Futex dtored while lock is held or pending?
		assert(storage.latch.load() == initial_value);
	}

	shared_futex_t(shared_futex_t &&) = delete;
	shared_futex_t(const shared_futex_t &) = delete;
	shared_futex_t &operator=(shared_futex_t &&) = delete;
	shared_futex_t &operator=(const shared_futex_t &) = delete;

	template <typename Locker>
	void lock() noexcept {
		this->template try_lock_until<Locker>(std::chrono::steady_clock::time_point::max());
	}

	template <typename Locker>
	bool try_lock() noexcept {
		return Locker().try_lock(*this);
	}

	template <typename Locker, typename Rep, typename Period>
	bool try_lock_for(const std::chrono::duration<Rep, Period> &duration) noexcept {
		return this->template try_lock_until<Locker>(std::chrono::steady_clock::now() + duration);
	}

	template <typename Locker, typename Clock, typename Duration>
	bool try_lock_until(const std::chrono::time_point<Clock, Duration> &time_point) noexcept {
		return Locker().try_lock_until(*this, time_point);
	}

	template <typename Locker>
	void unlock() noexcept {
		Locker().unlock(*this);
	}

	storage_type &data() noexcept { return storage; }
	const storage_type &data() const noexcept { return storage; }
};

template <typename SharedFutex, typename Locker>
class lock_guard {
	SharedFutex *l{ nullptr };
	Locker locker;

public:
	using mutex_type = SharedFutex;
	using locker_type = Locker;

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

	friend void swap(lock_guard<SharedFutex, Locker> &a, lock_guard<SharedFutex, Locker> &b) noexcept {
		std::swap(a.l, b.l);
		std::swap(a.locker, b.locker);
	}

	void swap(lock_guard &o) noexcept { swap(*this, o); }

	void lock() noexcept {
		try_lock_until(std::chrono::steady_clock::time_point::max());
	}

	bool try_lock() noexcept {
		assert(l);
		return locker.try_lock(*l);
	}

	template <typename Rep, typename Period>
	bool try_lock_for(const std::chrono::duration<Rep, Period> &duration) noexcept {
		const auto until = std::chrono::steady_clock::now() + duration;
		return try_lock_until(until);
	}

	template <typename Clock, typename Duration>
	bool try_lock_until(const std::chrono::time_point<Clock, Duration> &time_point) noexcept {
		assert(l);
		return locker.try_lock_until(*l, time_point);
	}

	void unlock() noexcept {
		assert(l);
		locker.unlock(*l);
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

template <typename Lock, typename BackoffPolicy, mechanism mec_>
class generic_locker {
public:
	using storage_policy = typename Lock::storage_policy;
	using latch_type = typename Lock::latch_type;

	static constexpr auto mchnsm = mec_;

protected:
	enum class lock_status : std::uint8_t {
		// Lock unacquired
		unacquired,
		// Lock unacquired and registered as a waiter
		waiter,
		// Lock unacquired, registered as a waiter and parked
		parked,
		// Lock successfully acquired
		acquired
	};

protected:
	lock_status status{ lock_status::unacquired };

	// Attempts to lock
	// If template parameter is true, also reverts wait registration
	template <bool unregister_wait = false>
	bool try_lock_impl(Lock &l) noexcept {
#ifdef SHARED_FUTEX_DEBUG
		if constexpr (unregister_wait)
			assert(status == lock_status::waiter);
		else
			assert(status == lock_status::unacquired);
#endif

		// Try to take lock and unregister
		auto bits = lock_bits();
		const auto waiter_bits = bits << Lock::waiters_counters_offset;
		if (unregister_wait)
			bits -= waiter_bits;

		const auto prev = l.data().latch.fetch_add(bits);

		status = valid(prev) ?
			lock_status::acquired :
			lock_status::unacquired;

#ifdef SHARED_FUTEX_STATS
		++debug_statistics.lock_rmw_instructions;
#endif

		return status == lock_status::acquired;
	}
	// Unlocks and registers us as a waiter in one rmw operation
	latch_type unlock_and_register_waiter(Lock &l) noexcept {
#ifdef SHARED_FUTEX_DEBUG
		assert(status == lock_status::unacquired);
#endif

		const auto locking_bits = lock_bits();
		const auto waiter_bits = locking_bits << Lock::waiters_counters_offset;
		const auto bits = waiter_bits - locking_bits;

		const auto new_value = l.data().latch.fetch_add(bits) + bits;

		on_unlock(l, new_value);
		status = lock_status::waiter;

#ifdef SHARED_FUTEX_STATS
		++debug_statistics.lock_rmw_instructions;
#endif

		return new_value;
	}
	// Registers the lock as a parked thread
	void register_parked(Lock &l) noexcept {
#ifdef SHARED_FUTEX_DEBUG
		assert(status == lock_status::waiter);
#endif

		const auto locking_bits = lock_bits();
		const auto parked_bits = locking_bits;

		l.data().parks.fetch_add(parked_bits);
		status = lock_status::parked;

#ifdef SHARED_FUTEX_STATS
		++debug_statistics.lock_rmw_instructions;
#endif
	}
	// Removes the lock from the parked and waiter counters
	void unregister_parked_and_waiter(Lock &l) noexcept {
#ifdef SHARED_FUTEX_DEBUG
		assert(status == lock_status::parked);
#endif

		const auto locking_bits = lock_bits();
		const auto parked_bits = locking_bits;
		const auto waiter_bits = locking_bits << Lock::waiters_counters_offset;

		l.data().parks.fetch_add(-parked_bits);
		l.data().latch.fetch_add(-waiter_bits);
		status = lock_status::unacquired;

#ifdef SHARED_FUTEX_STATS
		debug_statistics.lock_rmw_instructions += 2;
#endif
	}
	static void on_unlock(Lock &l, latch_type new_value) noexcept {
		BackoffPolicy().on_unlock(l, new_value, mchnsm);
	}

protected:
	// Decides whether the latch value, prior to a locking attempt, is valid for lock acquisition.
	virtual bool valid(latch_type) const noexcept = 0;
	// Specifies the bits used to perform the lock
	virtual latch_type lock_bits() const noexcept = 0;

	virtual std::pair<bool, latch_type> should_try_lock(Lock &l, std::memory_order mo) const noexcept {
#ifdef SHARED_FUTEX_STATS
		if (mo != std::memory_order_relaxed)
			++debug_statistics.lock_atomic_loads;
#endif

		auto val = l.data().latch.load(mo);
		return std::make_pair(valid(val), val);
	}
	virtual bool should_try_lock_backoff_predicate(Lock &l, std::memory_order mo) const noexcept { return should_try_lock(l, mo).first; }

public:
	virtual ~generic_locker() = default;

	virtual bool try_lock(Lock &l) noexcept {
		if (!try_lock_impl<>(l)) {
			// If attempt fails we need to revert.
			unlock(l);
			return false;
		}

		return true;
	}

	template <typename Clock, typename Duration>
	bool try_lock_until(Lock &l, const std::chrono::time_point<Clock, Duration> &until) noexcept {
		// Attempt lock. We assume this usually succeeds.
		if (try_lock_impl<>(l))
			return true;

#ifdef SHARED_FUTEX_STATS
		bool has_been_unparked = false;
#endif

		for (int iteration = 1;; ++iteration) {
			// If attempt fails revert lock attempt and register us as an active waiter
			auto new_lock_value = unlock_and_register_waiter(l);

			for (;; ++iteration) {
#ifdef SHARED_FUTEX_STATS
				++debug_statistics.iterations;
#endif

				// Once backoff policy decides to park us, register us as parked.
				const auto park_predicate = [&]() {
					// The parking mutex will provide an acquire fence
					return should_try_lock_backoff_predicate(l, std::memory_order_relaxed);
				};
				const auto on_park = [&]() {
					register_parked(l);
				};
				// Execute back-off policy
				const auto result = BackoffPolicy().backoff(l,
															new_lock_value,
															park_predicate,
															on_park,
															mchnsm,
															iteration,
															until);

				// If we have been parked then one of the following is possible:
				// - We have been signaled and unparked: Then we hold the lock, the unparker handles the lock acquisition for us, including 
				//   unregistering from wait and park counters.
				// - Predicate has been triggered, we can immediately try to retake lock.
				// - Timed-out: Need to unregister and fail.
				if (status == lock_status::parked) {
					if (result == backoff_result::unparked) {
						status = lock_status::acquired;
						return true;
					}
					
					if (result == backoff_result::park_predicate_triggered) {
						l.data().parks.fetch_add(-lock_bits());
						status = lock_status::waiter;

						// Try locking again, also unregisters us from waiter counters
						if (try_lock_impl<true>(l))
							return true;
						break;
					}

					// Timed-out
					unregister_parked_and_waiter(l);
					return false;
				}

				// On successful backoff we check, conservatively without ping-ponging cache lines, if we should reattempt to acquire lock.
				if (result == backoff_result::success) {
					const auto& p = should_try_lock(l, std::memory_order_acquire);
					if (p.first) {
						// Try locking again, also unregisters us from waiter counters
						if (try_lock_impl<true>(l))
							return true;
						break;
					}

					new_lock_value = p.second;
				}
			}

#ifdef SHARED_FUTEX_STATS
			++debug_statistics.in_loop_try_lock_failures;
#endif

		}
	}

	virtual void unlock(Lock &l) noexcept {
#ifdef SHARED_FUTEX_DEBUG
		assert(status == lock_status::acquired);
#endif

		const auto bits = -lock_bits();
		auto new_value = l.data().latch.fetch_add(bits) + bits;
		status = lock_status::unacquired;

#ifdef SHARED_FUTEX_STATS
		++debug_statistics.lock_rmw_instructions;
#endif

		on_unlock(l, new_value);
	}

	bool owns_lock() const noexcept { return status == lock_status::acquired; }
};

// shared_futex_detail
}

// Shared locker
template <typename Lock, typename BackoffPolicy>
class shared_locker final : public shared_futex_detail::generic_locker<
	Lock, BackoffPolicy, 
	shared_futex_detail::mechanism::shared_lock
> {
	latch_type lock_bits() const noexcept override final { return Lock::shared_lock_bit; }
	bool valid(latch_type l) const noexcept override final {
		return (l & Lock::non_shared_mask) == (Lock::initial_value & Lock::non_shared_mask);
	}
};

// Upgradeable locker
template <typename Lock, typename BackoffPolicy>
class upgradeable_locker final : public shared_futex_detail::generic_locker<
	Lock, BackoffPolicy,
	shared_futex_detail::mechanism::upgradeable_lock
> {
	latch_type lock_bits() const noexcept override final { return Lock::upgradeable_lock_bit; }
	bool valid(latch_type l) const noexcept override final {
		return (l & Lock::non_shared_mask) == (Lock::initial_value & Lock::non_shared_mask);
	}
};

// Exclusive locker
template <typename Lock, typename BackoffPolicy>
class exclusive_locker final : public shared_futex_detail::generic_locker<
	Lock, BackoffPolicy,
	shared_futex_detail::mechanism::exclusive_lock
> {
	latch_type lock_bits() const noexcept override final { return Lock::exclusive_lock_bit; }
	bool valid(latch_type l) const noexcept override final {
		return (l & Lock::all_locks_mask) == (Lock::initial_value & Lock::all_locks_mask);
	}
};

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

using shared_futex = shared_futex_t<shared_futex_default_storage_policy>;

template <typename BackoffPolicy, typename SharedFutex, typename... Args>
lock_guard<SharedFutex, shared_locker<SharedFutex, BackoffPolicy>> make_shared_lock(SharedFutex &l, Args &&... args) {
	return lock_guard<SharedFutex, shared_locker<SharedFutex, BackoffPolicy>>(l, std::forward<Args>(args)...);
}

template <typename BackoffPolicy, typename SharedFutex, typename... Args>
lock_guard<SharedFutex, upgradeable_locker<SharedFutex, BackoffPolicy>> make_upgradeable_lock(SharedFutex &l, Args &&... args) {
	return lock_guard<SharedFutex, upgradeable_locker<SharedFutex, BackoffPolicy>>(l, std::forward<Args>(args)...);
}

template <typename BackoffPolicy, typename SharedFutex, typename... Args>
lock_guard<SharedFutex, exclusive_locker<SharedFutex, BackoffPolicy>> make_exclusive_lock(SharedFutex &l, Args &&... args) {
	return lock_guard<SharedFutex, exclusive_locker<SharedFutex, BackoffPolicy>>(l, std::forward<Args>(args)...);
}

/*
*	@brief	Upgrades the lock owned by an upgradeable lock_guard to an exclusive lock, consuming the guard and returning an exclusive one.
*			Lock must have been successfully acquired via an upgrade lock.
*			
*	@param	upgradeable_guard	Must be a lock_guard using an upgradeable_locker and owning the lock
*/
template <typename BackoffPolicy, typename SharedFutex, typename B>
lock_guard<SharedFutex, exclusive_locker<SharedFutex, BackoffPolicy>> upgrade_lock(lock_guard<SharedFutex, upgradeable_locker<SharedFutex, B>> &&upgradeable_guard) {
	// Upgradeable guard owns lock?
	assert(upgradeable_guard.owns_lock());

	// Read lock and dispose of the upgradeable guard
	auto lock = &upgradeable_guard.mutex();
	upgradeable_guard.release();

	// Ugrade
//	lock_upgrader<SharedFutex, BackoffPolicy>().try_lock_until(*lock, std::chrono::steady_clock::time_point::max());

	// Return an exclusive guard owning the lock
	return lock_guard<SharedFutex, exclusive_locker<SharedFutex, BackoffPolicy>>(*lock, std::defer_lock);
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
std::pair<bool, lock_guard<SharedFutex, exclusive_locker<SharedFutex, BackoffPolicy>>> try_upgrade_lock_until(lock_guard<SharedFutex, upgradeable_locker<SharedFutex, B>> &&upgradeable_guard, const std::chrono::time_point<Clock, Duration> &until) {
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

	return std::make_pair(true, lock_guard<SharedFutex, exclusive_locker<SharedFutex, BackoffPolicy>>{});
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
std::pair<bool, lock_guard<SharedFutex, exclusive_locker<SharedFutex, BackoffPolicy>>> try_upgrade_lock_for(lock_guard<SharedFutex, upgradeable_locker<SharedFutex, B>> &&upgradeable_guard, const std::chrono::duration<Rep, Period> &duration) {
	const auto until = std::chrono::steady_clock::now() + duration;
	return try_upgrade_lock_until<BackoffPolicy>(std::move(upgradeable_guard), until);
}

}
