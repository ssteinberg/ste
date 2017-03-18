// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#ifdef _MSC_VER
#include <windows.hpp>
#elif defined _linux
#include <sched.h>
#include <pthread.h>
#include <functional>
#else
#error Unsupported OS
#endif

#include <thread>
#include <bitset>

namespace StE {

template <int N>
bool inline thread_set_affinity(std::thread *thread, const std::bitset<N> &mask) {
	static_assert(N <= sizeof(std::size_t) * 8, "Affinity mask can't exceed native WORD size.");

	auto handle = thread->native_handle();

#ifdef _MSC_VER

	DWORD m = 0;
	if (sizeof(DWORD) == sizeof(decltype(mask.to_ulong())))
		m = static_cast<DWORD>(mask.to_ulong());
	else if (sizeof(DWORD) == sizeof(decltype(mask.to_ullong())))
		m = static_cast<DWORD>(mask.to_ullong());
	else
		assert(false);

	return SetThreadAffinityMask(handle, m) != 0;

#elif defined _linux

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);

	for (int i = 0; i < N; ++i)
		if (mask[i])
			CPU_SET(i, &cpuset);

	return pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset) == 0;

#endif
}

}
