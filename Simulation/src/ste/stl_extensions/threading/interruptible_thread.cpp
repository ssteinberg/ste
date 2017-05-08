
#include <stdafx.hpp>
#include <interruptible_thread.hpp>

thread_local ste::interruptible_thread::interruptible_thread_flag *ste::interruptible_thread::interruption_flag;
