
#include <stdafx.hpp>
#include <ste_device_queue_selector_cache.hpp>

using namespace ste::gl;

thread_local ste_device_queue_selector_cache::cache_t ste_device_queue_selector_cache::cached_usage_index_map;
