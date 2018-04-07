// StE
// © Shlomi Steinberg, 2015-2018

#pragma once

#define SHARED_FUTEX_DEBUG
//#define SHARED_FUTEX_STATS

namespace ste::shared_futex_detail {

#ifdef SHARED_FUTEX_STATS

struct statistics {
	std::chrono::high_resolution_clock::duration unparking_time{};
	std::chrono::high_resolution_clock::duration spinning_time{};

	std::size_t iterations{};
	std::size_t in_loop_try_lock_failures{};
	std::size_t lock_rmw_instructions{};
	std::size_t lock_atomic_loads{};

	std::size_t lock_parks{};
	std::size_t lock_multiple_parks{};
	std::size_t unparks{};
};

// Statistics collected per thread
static thread_local statistics debug_statistics;

#endif

enum class mechanism {
	shared_lock,
	upgradeable_lock,
	exclusive_lock,
	upgrading_to_exclusive_lock,
	pending_exclusive_lock,
};

enum class backoff_result {
	unparked,
	park_predicate_triggered,
	timeout,
	success,
};

}
