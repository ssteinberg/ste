// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "windows.h"

#include <thread>

namespace StE {

bool inline thread_set_priority_low(std::thread *thread) {
	auto handle = thread->native_handle();
	return SetThreadPriority(handle, THREAD_PRIORITY_BELOW_NORMAL);
}

bool inline thread_set_priority_high(std::thread *thread) {
	auto handle = thread->native_handle();
	return SetThreadPriority(handle, THREAD_PRIORITY_ABOVE_NORMAL);
}

}
