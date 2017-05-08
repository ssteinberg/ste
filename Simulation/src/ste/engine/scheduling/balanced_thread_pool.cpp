
#include <stdafx.hpp>
#include <balanced_thread_pool.hpp>

using namespace ste;

thread_local bool balanced_thread_pool::balanced_thread_pool_worker_thread_flag = false;
