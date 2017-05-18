
#include <stdafx.hpp>
#include <lib/allocator.hpp>

using namespace ste;

lib::_internal::_allocator_rpmalloc_init lib::allocator_base::_rpmalloc_init;
thread_local lib::_internal::_allocator_rpmalloc_thread_init lib::allocator_base::_rpmalloc_thread_init;
