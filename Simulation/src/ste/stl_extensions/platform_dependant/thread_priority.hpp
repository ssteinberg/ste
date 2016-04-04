// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#ifdef _MSC_VER
#include "windows.hpp"
#elif defined _linux
#include "optional.hpp"
#include <pthread.h>
#else
#error Unsupported OS
#endif

#include <thread>

namespace StE {
	
namespace detail {
	
#ifdef _linux

inline bool set_thread_sched(const pthread_t &t, int policy, int priority) {
	int p, s;
	struct sched_param param;

	s = pthread_getschedparam(t, &p, &param);
	assert(s == 0);
	
	if (s != 0)
		return false;
	
	param.sched_priority = priority;
	s = pthread_setschedparam(t, policy, &param);
	assert(s == 0);
	
	return s == 0;
}

#endif 
	
}

bool inline thread_set_priority_idle(std::thread *thread) {
	auto handle = thread->native_handle();
#ifdef _MSC_VER
	return SetThreadPriority(handle, THREAD_PRIORITY_IDLE);
#elif defined _linux
	return detail::set_thread_sched(handle, SCHED_IDLE, 0); 
#endif
}

bool inline thread_set_priority_low(std::thread *thread) {
	auto handle = thread->native_handle();
#ifdef _MSC_VER
	return SetThreadPriority(handle, THREAD_PRIORITY_BELOW_NORMAL);
#elif defined _linux
	return detail::set_thread_sched(handle, SCHED_BATCH, 0);
#endif
}

bool inline thread_set_priority_normal(std::thread *thread) {
	auto handle = thread->native_handle();
#ifdef _MSC_VER
	return SetThreadPriority(handle, THREAD_PRIORITY_NORMAL);
#elif defined _linux
	return detail::set_thread_sched(handle, SCHED_OTHER, 0);
#endif
}

bool inline thread_set_priority_high(std::thread *thread) {
	auto handle = thread->native_handle();
#ifdef _MSC_VER
	return SetThreadPriority(handle, THREAD_PRIORITY_ABOVE_NORMAL);
#elif defined _linux
	return detail::set_thread_sched(handle, SCHED_RR, 0);
#endif
}

bool inline thread_set_priority_rtime(std::thread *thread) {
	auto handle = thread->native_handle();
#ifdef _MSC_VER
	return SetThreadPriority(handle, THREAD_PRIORITY_TIME_CRITICAL);
#elif defined _linux
	return detail::set_thread_sched(handle, SCHED_RR, 32);
#endif
}

}
