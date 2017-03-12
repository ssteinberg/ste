//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource_traits.hpp>

#include <vk_resource.hpp>
#include <device_resource_memory_allocator.hpp>

#include <functional>

namespace StE {
namespace GL {

template <typename T, class allocation_policy>
class device_resource : ste_resource_deferred_create_trait {
	static_assert(std::is_base_of<vk_resource, T>::value,
				  "T must be a vk_resource derived type");

private:
	struct ctor {};

public:
	using allocation_t = device_memory_heap::allocation_type;

private:
	T resource;
	allocation_t allocation;

private:
	device_resource(ctor,
					const ste_context &ctx,
					T &&resource)
		: resource(std::move(resource))
	{
		allocation = device_resource_memory_allocator<allocation_policy>()(ctx.device_memory_allocator(),
																		   resource);
	}

public:
	template <typename ... Args>
	device_resource(const ste_context &ctx,
					Args&&... args)
		: device_resource(ctor(), ctx, T(ctx.device().logical_device(), std::forward<Args>(args)...))
	{}
	~device_resource() noexcept {}

	auto& get_underlying_memory() { return *this->allocation.get_memory(); }
	auto& get_underlying_memory() const { return *this->allocation.get_memory(); }
	bool has_private_underlying_memory() const { return allocation.is_private_allocation(); }

	operator T&() { return get(); }
	operator const T&() const { return get(); }

	T& get() { return resource; }
	T& get() const { return resource; }

	T& operator->() { return get(); }
	T& operator->() const { return get(); }
	T& operator*() { return get(); }
	T& operator*() const { return get(); }
};

}
}
