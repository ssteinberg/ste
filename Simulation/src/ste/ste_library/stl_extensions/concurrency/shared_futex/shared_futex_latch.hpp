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
	using latch_data_type = typename storage_policy::latch_data_type;
	using parks_counter_type = typename storage_policy::parks_counter_type;

	static_assert(std::is_signed_v<latch_data_type> && std::is_signed_v<parks_counter_type>, "Latch and parks counter need to be signed types.");

	static constexpr latch_data_type initial_value = storage_policy::initial_value;
	static constexpr auto alignment = std::max(storage_policy::alignment, alignof(std::max_align_t));
	
	// Bit depth for futex upgrade-to-exclusive flag
	static constexpr std::size_t upgrade_to_exclusive_flag_bits = 1;
	// Bit depth for futex exclusive-priority boosting flag (to counter exclusive starvation)
	static constexpr std::size_t boost_flag_bits = 1;

	class latch_descriptor {
		friend class shared_futex_default_latch<StoragePolicy>;
		using T = std::make_unsigned_t<latch_data_type>;

		T shared_consumers					: storage_policy::shared_bits;
		T upgradeable_consumers				: storage_policy::upgradeable_bits;
		T exclusive_consumers				: storage_policy::exclusive_bits;

		T upgradeable_waiters				: storage_policy::upgradeable_bits;
		T exclusive_waiters					: storage_policy::exclusive_bits;

		T upgrade_to_exclusive_flag_value	: upgrade_to_exclusive_flag_bits;

		T boost_flag_value					: boost_flag_bits;

		explicit latch_descriptor(const latch_data_type &l) { *this = *reinterpret_cast<const latch_descriptor*>(&l); }
		explicit operator latch_data_type() const {
			latch_data_type t = {};
			*reinterpret_cast<latch_descriptor*>(&t) = *this;
			return t;
		}

		// Accessors and helpers

		template <shared_futex_detail::mechanism mechanism>
		void inc_consumers(const T &count) noexcept {
			switch (mechanism) {
			case shared_futex_detail::mechanism::shared_lock:
				shared_consumers += count;
				break;
			case shared_futex_detail::mechanism::upgradeable_lock:
				upgradeable_consumers += count;
				break;
			case shared_futex_detail::mechanism::exclusive_lock:
				exclusive_consumers += count;
				break;
			default:
				assert(false);
			}
		}
		template <shared_futex_detail::mechanism mechanism>
		void inc_waiters(const T &count) noexcept {
			assert(mechanism == shared_futex_detail::mechanism::exclusive_lock || 
				   mechanism == shared_futex_detail::mechanism::upgradeable_lock);

			switch (mechanism) {
			case shared_futex_detail::mechanism::upgradeable_lock:
				upgradeable_waiters += count;
				break;
			case shared_futex_detail::mechanism::exclusive_lock:
				exclusive_waiters += count;
				break;
			default:{}
			}
		}

	public:
		latch_descriptor() = default;

		// Counts number of active consumers
		template <shared_futex_detail::mechanism mechanism>
		auto consumers() const noexcept {
			switch (mechanism) {
			case shared_futex_detail::mechanism::shared_lock:
				return shared_consumers;
			case shared_futex_detail::mechanism::upgradeable_lock:
				return upgradeable_consumers;
			case shared_futex_detail::mechanism::exclusive_lock:
				return exclusive_consumers;
			default:
				assert(false);
				return T{};
			}
		}
		// Counts number of waiting consumers
		template <shared_futex_detail::mechanism mechanism>
		auto waiters() const noexcept {
			static_assert(mechanism == shared_futex_detail::mechanism::exclusive_lock || 
						  mechanism == shared_futex_detail::mechanism::upgradeable_lock);
			switch (mechanism) {
			case shared_futex_detail::mechanism::upgradeable_lock:
				return upgradeable_waiters;
			case shared_futex_detail::mechanism::exclusive_lock:
			default:
				return exclusive_waiters;
			}
		}
		auto boost_flag() const noexcept { return boost_flag_value; }
		auto upgrade_to_exclusive_flag() const noexcept { return upgrade_to_exclusive_flag_value; }
	};
	class parks_descriptor {
		friend class shared_futex_default_latch<StoragePolicy>;
		using T = std::make_unsigned_t<parks_counter_type>;

		T shared_parked					: storage_policy::shared_bits;
		T upgradeable_parked			: storage_policy::upgradeable_bits;
		T exclusive_parked				: storage_policy::exclusive_bits;
		T upgrading_to_exclusive_parked : 1;

		explicit parks_descriptor(const parks_counter_type &l) { *this = *reinterpret_cast<const parks_descriptor*>(&l); }
		explicit operator parks_counter_type() const {
			parks_counter_type c = {};
			*reinterpret_cast<parks_descriptor*>(&c) = *this;
			return c;
		}

		// Accessors and helpers

		template <shared_futex_detail::mechanism mechanism>
		void inc_parked(const T &count) noexcept {
			switch (mechanism) {
			case shared_futex_detail::mechanism::shared_lock:
				shared_parked += count;
				break;
			case shared_futex_detail::mechanism::upgradeable_lock:
				upgradeable_parked += count;
				break;
			case shared_futex_detail::mechanism::exclusive_lock:
				exclusive_parked += count;
				break;
			case shared_futex_detail::mechanism::upgrade_to_exclusive_lock:
				upgrading_to_exclusive_parked += count;
				break;
			default:{}
			}
		}

	public:
		parks_descriptor() = default;

		// Counts number of parked consumers
		template <shared_futex_detail::mechanism mechanism>
		auto parked() const noexcept {
			switch (mechanism) {
			case shared_futex_detail::mechanism::shared_lock:
				return shared_parked;
			case shared_futex_detail::mechanism::upgradeable_lock:
				return upgradeable_parked;
			case shared_futex_detail::mechanism::exclusive_lock:
				return exclusive_parked;
			case shared_futex_detail::mechanism::upgrade_to_exclusive_lock:
			default:
				return upgrading_to_exclusive_parked;
			}
		}
	};

private:
	using latch_atomic_t = std::atomic<latch_data_type>;
	using parks_atomic_t = std::atomic<parks_counter_type>;

	static_assert(sizeof(latch_descriptor) <= sizeof(latch_data_type), "Total bits count should take no more than the latch size");
	static_assert(sizeof(parks_descriptor) <= sizeof(parks_counter_type), "Total bits count should take no more than the parks counter size");
	static_assert(latch_atomic_t::is_always_lock_free, "Latch is not lock-free!");
	static_assert(parks_atomic_t::is_always_lock_free, "Latch parking counter is not lock-free!");

private:
	// Latch
	alignas(alignment) latch_atomic_t latch{ initial_value };
	// Parking counters
	alignas(alignment) parks_atomic_t parks{};

public:
	// Parking lot for smart wakeup
	parking_lot<void> parking;

public:
	shared_futex_default_latch() = default;
	~shared_futex_default_latch() noexcept {
		// Latch dtored while lock is held or pending?
		assert(latch.load() == initial_value);
	}

	shared_futex_default_latch(shared_futex_default_latch&&) = default;
	shared_futex_default_latch &operator=(shared_futex_default_latch&&) = default;

	shared_futex_default_latch(const shared_futex_default_latch&) = delete;
	shared_futex_default_latch &operator=(const shared_futex_default_latch&) = delete;

	latch_descriptor load(std::memory_order mo = std::memory_order_acquire) const noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_atomic_loads;
#endif
		return latch_descriptor{ latch.load(mo) };
	}
	parks_descriptor load_parked(std::memory_order mo = std::memory_order_acquire) const noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_atomic_loads;
#endif
		return parks_descriptor{ parks.load(mo) };
	}
	
	// Attempts to acquire lock
	template <shared_futex_detail::mechanism mechanism>
	latch_descriptor acquire() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		// Attempts lock acquisition
		latch_descriptor d = {};
		d.template inc_consumers<mechanism>(1);
		const auto bits = static_cast<latch_data_type>(d);
		return latch_descriptor{ latch.fetch_add(bits) };
	}
	// Re-attempts lock acquisition and decreases waiter counter
	template <shared_futex_detail::mechanism mechanism>
	latch_descriptor reattempt_acquire() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		latch_descriptor d1 = {}, d2 = {};
		d1.template inc_consumers<mechanism>(1);
		if constexpr (mechanism == shared_futex_detail::mechanism::exclusive_lock || mechanism == shared_futex_detail::mechanism::upgradeable_lock)
			d2.template inc_waiters<mechanism>(1);
		const auto bits = static_cast<latch_data_type>(d1) - static_cast<latch_data_type>(d2);
		return latch_descriptor{ latch.fetch_add(bits) };
	}
	// Reverts lock acquisition and increases waiter counter
	template <shared_futex_detail::mechanism mechanism>
	latch_descriptor revert() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		latch_descriptor d1 = {}, d2 = {};
		d1.template inc_consumers<mechanism>(1);
		if constexpr (mechanism == shared_futex_detail::mechanism::exclusive_lock || mechanism == shared_futex_detail::mechanism::upgradeable_lock)
			d2.template inc_waiters<mechanism>(1);
		const auto bits = static_cast<latch_data_type>(d2) - static_cast<latch_data_type>(d1);
		return latch_descriptor{ latch.fetch_add(bits) + bits };
	}
	// Release lock
	template <shared_futex_detail::mechanism mechanism>
	latch_descriptor release() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		// Decrease comsumer count
		latch_descriptor d = {};
		d.template inc_consumers<mechanism>(1);
		const auto bits = -static_cast<latch_data_type>(d);
		return latch_descriptor{ latch.fetch_add(bits) + bits };
	}

	// Upgrades an upgradeable lock to an exclusive lock
	latch_descriptor upgrade() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		// Attempts lock upgrade, sets the upgrade-to-exclusive flag, increases exclusive count and releases the upgradeable consumer.
		latch_descriptor d1 = {}, d2 = {};
		d1.upgrade_to_exclusive_flag_value = 1;
		d1.template inc_consumers<shared_futex_detail::mechanism::exclusive_lock>(1);
		d2.template inc_consumers<shared_futex_detail::mechanism::upgradeable_lock>(1);
		const auto bits = static_cast<latch_data_type>(d1) - static_cast<latch_data_type>(d2);
		return latch_descriptor{ latch.fetch_add(bits) };
	}
	// Reverts lock upgrade
	latch_descriptor revert_upgrade() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		// Reverts lock upgrade, resets the upgrade-to-exclusive flag, releases the exclusive consumer and increases the upgradeable consumers counter.
		latch_descriptor d1 = {}, d2 = {};
		d1.upgrade_to_exclusive_flag_value = 1;
		d1.template inc_consumers<shared_futex_detail::mechanism::exclusive_lock>(1);
		d2.template inc_consumers<shared_futex_detail::mechanism::upgradeable_lock>(1);
		const auto bits = static_cast<latch_data_type>(d2) - static_cast<latch_data_type>(d1);
		return latch_descriptor{ latch.fetch_add(bits) + bits };
	}
	// Release an upgraded lock
	latch_descriptor release_upgraded() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		// Reverts lock upgrade, resets the upgrade-to-exclusive flag and increases the upgradeable holder.
		latch_descriptor d = {};
		d.upgrade_to_exclusive_flag_value = 1;
		d.template inc_consumers<shared_futex_detail::mechanism::exclusive_lock>(1);
		const auto bits = -static_cast<latch_data_type>(d);
		return latch_descriptor{ latch.fetch_add(bits) + bits };
	}

	// Raises the boost flag
	void set_boost_flag(std::size_t value = 1) noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		// Set the boost flag
		latch_descriptor d = {};
		d.boost_flag_value = value;
		const auto bits = static_cast<latch_data_type>(d);
		latch.fetch_or(bits);
	}
	// Resets the boost flag
	void reset_boost_flag() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		// Atomically AND to unset the flag
		latch_descriptor d = {};
		d.boost_flag_value = 1;
		const auto bits = ~static_cast<latch_data_type>(d);
		latch.fetch_and(bits);
	}

	// Generates a parking key which is unique per-mechanism and a 32-bit user value.
	template <shared_futex_detail::mechanism mechanism>
	static std::uint64_t parking_key(const std::uint32_t user_value) noexcept {
		return static_cast<uint64_t>(user_value) | (static_cast<uint64_t>(mechanism) << 32);
	}

	// Register parked thread
	template <shared_futex_detail::mechanism mechanism>
	void register_parked() noexcept {
#ifdef SHARED_FUTEX_STATS
		++shared_futex_detail::debug_statistics.lock_rmw_instructions;
#endif
		parks_descriptor d = {};
		d.template inc_parked<mechanism>(1);
		const auto bits = static_cast<parks_counter_type>(d);
		parks.fetch_add(bits);
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
		parks_descriptor d = {};
		d.template inc_parked<mechanism>(count);
		const auto bits = -static_cast<parks_counter_type>(d);
		parks.fetch_add(bits);
	}
};

}
