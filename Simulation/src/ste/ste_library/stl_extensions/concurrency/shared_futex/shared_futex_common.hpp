// StE
// © Shlomi Steinberg, 2015-2018

#pragma once

// #define SHARED_FUTEX_DEBUG
// #define SHARED_FUTEX_STATS

namespace ste::shared_futex_detail {

#ifdef SHARED_FUTEX_STATS

struct statistics {
	std::size_t iterations{};
	std::size_t lock_rmw_instructions{};
	std::size_t lock_atomic_loads{};

	std::size_t lock_parks{};
	std::size_t unparks{};
};

// Statistics collected per thread
static thread_local statistics debug_statistics;

#endif


enum class lock_status : std::uint8_t {
	// Lock unacquired
	unacquired,
	// Waiting for lock acquisition
	waiting,
	// Thread parked
	parked,
	// Lock successfully acquired
	acquired
};

enum class modus_operandi : std::uint8_t {
	shared_lock,
	upgradeable_lock,
	exclusive_lock,
	upgrade_to_exclusive_lock,
};

enum class backoff_result : std::uint8_t {
	unparked,
	park_predicate_triggered,
	timeout,
	success,
};
enum class unpark_operation : std::uint8_t {
	none,
	unpark,
	reserve_and_unpark,
};

enum class backoff_aggressiveness : std::uint8_t {
	aggressive,
	normal,
	relaxed,
	very_relaxed,
};

struct backoff_return_t {
	backoff_result result;
	unpark_operation unpark_op{ unpark_operation::none };
};

}
