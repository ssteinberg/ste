// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "windows.h"

#include <thread>

namespace StE {

bool inline thread_set_priority_low(std::thread *thread) {
	auto handle = thread->native_handle();
#ifdef _MSC_VER
	return SetThreadPriority(handle, THREAD_PRIORITY_BELOW_NORMAL);
#else
#error Unsupported
#endif
}

bool inline thread_set_priority_high(std::thread *thread) {
	auto handle = thread->native_handle();
#ifdef _MSC_VER
	return SetThreadPriority(handle, THREAD_PRIORITY_ABOVE_NORMAL);
#else
#error Unsupported
#endif
}

}
