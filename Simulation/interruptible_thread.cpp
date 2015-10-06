
#include "stdafx.h"
#include "interruptible_thread.h"

thread_local StE::interruptible_thread::interruptible_thread_flag *StE::interruptible_thread::interruption_flag;
