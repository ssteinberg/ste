// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <thread>

namespace ste {

extern std::thread::id main_thread_id;

bool inline is_main_thread() { return std::this_thread::get_id() == main_thread_id; }

}
