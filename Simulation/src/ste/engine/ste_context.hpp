//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <ste_gl_context.hpp>
#include <ste_engine.hpp>
#include <vk_exception.hpp>

namespace StE {

template <typename Types>
class ste_context_impl {
public:
	using context_types = Types;

	using gl_device_t = ste_engine::gl_device_t;
	using gl_context_t = ste_engine::gl_context_t;

private:
	ste_engine_impl<Types> &engine_reference;
	const gl_context_t &gl_context;
	gl_device_t &gl_device;
	typename context_types::gl_device_memory_allocator engine_device_memory_allocator;

public:
	ste_context_impl(ste_engine_impl<Types> &engine,
					 const gl_context_t &gl_ctx,
					 gl_device_t &device)
		: engine_reference(engine),
		gl_context(gl_ctx),
		gl_device(device),
		engine_device_memory_allocator(device.logical_device())
	{}
	~ste_context_impl() noexcept {}

	/**
	*	@brief	Performs schedules work.
	*
	*	@throws ste_engine_exception		On internal error
	*	@throws ste_device_exception		On internal device error
	*	@throws vk_exception				On Vulkan device error
	*	@throws ste_engine_glfw_exception	On windowing system error
	*/
	void tick() {
		engine_reference.tick();
		gl_device.tick();
	}

	ste_context_impl(ste_context_impl &&) = default;
	ste_context_impl(const ste_context_impl &) = delete;
	ste_context_impl &operator=(ste_context_impl &&) = default;
	ste_context_impl &operator=(const ste_context_impl &) = delete;

	ste_engine_impl<Types> &engine() const { return engine_reference; }
	gl_device_t &device() const { return gl_device; }
	auto &gl() const { return gl_context; }
	auto &device_memory_allocator() const { return engine_device_memory_allocator; }
};

using ste_context = ste_context_impl<
	ste_engine_types
>;

}
