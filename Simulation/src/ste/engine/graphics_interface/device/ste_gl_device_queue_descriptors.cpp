
#include <stdafx.hpp>
#include <ste_gl_device_queue_descriptors.hpp>

using namespace StE::GL;

thread_local ste_gl_queue_descriptors::cache_t ste_gl_queue_descriptors::cached_usage_index_map;
