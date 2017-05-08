
#include <stdafx.hpp>
#include <ste_device_pipeline_cache.hpp>

#include <vector>

using namespace ste::gl;

void ste_device_pipeline_cache::read_origin() {
	// Create origin from stored data in the non-volatile cache
	auto optional = non_volatile_cache->get<std::string>(std::string(non_volatile_cache_key_prefix) + device_name)();
	std::string origin_data = optional ? optional.get() : std::string();
	origin = std::make_unique<vk::vk_pipeline_cache>(device.get(), origin_data);
}

void ste_device_pipeline_cache::store_all_caches() {
	std::vector<pipeline_cache_ptr_t> caches;

	// Get all created caches
	auto cache = created_caches.pop();
	while (cache != nullptr) {
		caches.push_back(std::move(cache));
		cache = created_caches.pop();
	}

	// Merge and write out
	if (caches.size()) {
		std::vector<std::reference_wrapper<const pipeline_cache_t>> refs;
		for (auto &c : caches)
			refs.emplace_back(std::ref(*c));

		pipeline_cache_t new_origin(device.get(), refs);
		std::string new_origin_data = new_origin.read_raw_cache_data();

		non_volatile_cache->insert(std::string(non_volatile_cache_key_prefix) + device_name,
								   new_origin_data);
	}
}

const ste_device_pipeline_cache::pipeline_cache_t& ste_device_pipeline_cache::current_thread_cache() const {
	// Try to get an existing cache for current thread
	auto tid = std::this_thread::get_id();
	{
		auto guard = thread_cache_map[tid];
		if (guard.is_valid())
			return **guard;
	}

	// Create new cache object
	std::vector<std::reference_wrapper<const pipeline_cache_t>> origin_ref = { *origin };
	pipeline_cache_ptr_t new_cache = std::make_unique<pipeline_cache_t>(device.get(), 
																		origin_ref);
	auto *ptr = new_cache.get();
	created_caches.push(std::move(new_cache));
	thread_cache_map.emplace(tid, ptr);

	return *ptr;
}
