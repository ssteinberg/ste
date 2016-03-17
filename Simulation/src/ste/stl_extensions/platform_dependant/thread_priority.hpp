// StE
// � Shlomi Steinberg, 2015

#pragma once

#ifdef _MSC_VER
#include "windows.hpp"
#elif defined _linux
#include <pthread.h>
#else
#error Unsupported OS
#endif

#include <thread>

namespace StE {

bool inline thread_set_priority_low(std::thread *thread) {
	auto handle = thread->native_handle();
#ifdef _MSC_VER
	return SetThreadPriority(handle, THREAD_PRIORITY_BELOW_NORMAL);
#elif defined _linux
	return pthread_setschedprio(handle, 1) == 0; 
#endif
}

bool inline thread_set_priority_high(std::thread *thread) {
	auto handle = thread->native_handle();
#ifdef _MSC_VER
	return SetThreadPriority(handle, THREAD_PRIORITY_ABOVE_NORMAL);
#elif defined _linux
	return pthread_setschedprio(handle, 1) == 0;
#endif
}

}
