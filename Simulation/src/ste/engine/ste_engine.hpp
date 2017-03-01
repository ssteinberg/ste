//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <task_scheduler.hpp>
#include <lru_cache.hpp>
#include <ste_engine_storage_protocol.hpp>

#include <ste_presentation_device.hpp>
#include <ste_gl_context.hpp>
#include <ste_gl_device_queues_protocol.hpp>
#include <ste_gl_device_memory_allocator.hpp>

#include <ste_engine_exceptions.hpp>

namespace StE {

struct ste_engine_types {
	using task_scheduler_t = task_scheduler;
	using cache_t = lru_cache<std::string>;

	using storage_protocol = ste_engine_storage_protocol;
	using gl_device_memory_allocator = GL::ste_gl_device_memory_allocator;
	using gl_queues_protocol = GL::ste_gl_device_queues_protocol;
};

template <typename Types>
class ste_engine_impl {
public:
	using engine_types = Types;

	using gl_device_t = GL::ste_presentation_device<typename engine_types::gl_queues_protocol>;
	using gl_context_t = GL::ste_gl_context;
	using storage = typename engine_types::storage_protocol;

private:
	typename engine_types::task_scheduler_t engine_task_scheduler;
	typename engine_types::cache_t engine_cache;

	const gl_context_t &gl_context;
	const gl_device_t &gl_device;
	typename engine_types::gl_device_memory_allocator engine_device_memory_allocator;

public:
	ste_engine_impl(const gl_context_t &gl_ctx,
					const gl_device_t &device)
		: engine_cache(storage::cache_dir_path(), 1024 * 1024 * 256),
		gl_context(gl_ctx),
		gl_device(device),
		engine_device_memory_allocator(device.device())
	{}
	
	~ste_engine_impl() noexcept {}

	ste_engine_impl(ste_engine_impl &&) = default;
	ste_engine_impl(const ste_engine_impl &) = default;
	ste_engine_impl &operator=(ste_engine_impl &&) = default;
	ste_engine_impl &operator=(const ste_engine_impl &) = default;

	auto &task_scheduler() { return engine_task_scheduler; }
	auto &task_scheduler() const { return engine_task_scheduler; }
	auto &cache() { return engine_cache; }
	auto &cache() const { return engine_cache; }
	auto &gl() const { return gl_context; }
	auto &device() const { return gl_device; }
	auto &device_memory_allocator() const { return engine_device_memory_allocator; }
};

using ste_engine = ste_engine_impl<
	ste_engine_types
>;

}
