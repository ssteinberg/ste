
#include <stdafx.hpp>
#include <ste_device_queue_selector_cache.hpp>

using namespace StE::GL;

thread_local ste_device_queue_selector_cache::cache_t ste_device_queue_selector_cache::cached_usage_index_map;
