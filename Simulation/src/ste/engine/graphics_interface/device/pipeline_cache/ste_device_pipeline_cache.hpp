//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_engine.hpp>
#include <vk_pipeline_cache.hpp>
#include <vk_logical_device.hpp>

#include <concurrent_queue.hpp>
#include <concurrent_unordered_map.hpp>

#include <memory>
#include <thread>
#include <string>

namespace ste {
namespace gl {

class ste_device_pipeline_cache {
private:
	static constexpr char *non_volatile_cache_key_prefix = "ste_device_pipeline_cache_";

	using cache_t = ste_engine::engine_types::cache_t;

	using thread_id_t = std::thread::id;
	using pipeline_cache_t = vk::vk_pipeline_cache;

	using created_caches_queue_t = concurrent_queue<pipeline_cache_t>;

	using pipeline_cache_ptr_t = created_caches_queue_t::stored_ptr;

private:
	std::reference_wrapper<const vk::vk_logical_device> device;
	std::string device_name;

	cache_t *non_volatile_cache;
	pipeline_cache_ptr_t origin;

	mutable concurrent_unordered_map<thread_id_t, const vk::vk_pipeline_cache*> thread_cache_map;
	mutable created_caches_queue_t created_caches;

private:
	void read_origin();
	void store_all_caches();

public:
	ste_device_pipeline_cache(const vk::vk_logical_device &device,
							  cache_t *non_volatile_cache,
							  const std::string &device_name)
		: device(device),
		device_name(device_name),
		non_volatile_cache(non_volatile_cache)
	{
		read_origin();
	}
	~ste_device_pipeline_cache() noexcept {
		store_all_caches();
	}

	ste_device_pipeline_cache(ste_device_pipeline_cache&&) = default;
	ste_device_pipeline_cache &operator=(ste_device_pipeline_cache&&) = default;

	const pipeline_cache_t& current_thread_cache() const;
};

}
}