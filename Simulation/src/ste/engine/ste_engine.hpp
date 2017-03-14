//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <task_scheduler.hpp>
#include <lru_cache.hpp>
#include <ste_engine_storage_protocol.hpp>

#include <ste_device.hpp>
#include <ste_gl_context.hpp>
#include <ste_gl_device_memory_allocator.hpp>

#include <ste_engine_exceptions.hpp>

namespace StE {

struct ste_engine_types {
	using task_scheduler_t = task_scheduler;
	using cache_t = lru_cache<std::string>;

	using storage_protocol = ste_engine_storage_protocol;
	using gl_device_memory_allocator = GL::ste_gl_device_memory_allocator;
};

template <typename Types>
class ste_engine_impl {
public:
	using engine_types = Types;

	using gl_device_t = GL::ste_device;
	using gl_context_t = GL::ste_gl_context;
	using storage_protocol = typename engine_types::storage_protocol;

	static constexpr std::size_t cache_quota_size_bytes = 1024 * 1024 * 256;

private:
	typename engine_types::task_scheduler_t engine_task_scheduler;
	typename engine_types::cache_t engine_cache;

public:
	ste_engine_impl()
		: engine_cache(storage().cache_dir_path(), cache_quota_size_bytes)
	{}
	~ste_engine_impl() noexcept {}

	ste_engine_impl(ste_engine_impl &&) = default;
	ste_engine_impl(const ste_engine_impl &) = delete;
	ste_engine_impl &operator=(ste_engine_impl &&) = default;
	ste_engine_impl &operator=(const ste_engine_impl &) = delete;

	void tick() {
		engine_task_scheduler.tick();
	}

	auto &task_scheduler() { return engine_task_scheduler; }
	auto &task_scheduler() const { return engine_task_scheduler; }
	auto &cache() { return engine_cache; }
	auto &cache() const { return engine_cache; }
	storage_protocol storage() const { return storage_protocol(); }
};

using ste_engine = ste_engine_impl<
	ste_engine_types
>;

}
