//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource_traits.hpp>

#include <vk_resource.hpp>
#include <device_resource_memory_allocator.hpp>

#include <allow_type_decay.hpp>

namespace StE {
namespace GL {

template <typename T, class allocation_policy>
class device_resource
	: ste_resource_deferred_create_trait,
	public allow_type_decay<device_resource<T, allocation_policy>, T, false>
{
	static_assert(std::is_base_of<vk_resource, T>::value,
				  "T must be a vk_resource derived type");

protected:
	struct ctor {};

public:
	using resource_t = T;
	using allocation_t = device_memory_heap::allocation_type;

private:
	const ste_context &ctx;
	T resource;
	allocation_t allocation;

private:
	void allocate_memory() {
		allocation = device_resource_memory_allocator<allocation_policy>()(ctx.device_memory_allocator(),
																		   resource);
	}

protected:
	device_resource(ctor,
					const ste_context &ctx,
					T &&resource)
		: ctx(ctx),
		resource(std::move(resource))
	{
		allocate_memory();
	}

public:
	template <typename ... Args>
	device_resource(const ste_context &ctx,
					Args&&... args)
		: device_resource(ctor(),
						  ctx,
						  T(ctx.device(), std::forward<Args>(args)...))
	{}
	virtual ~device_resource() noexcept {}

	device_resource(device_resource&&) = default;
	device_resource &operator=(device_resource&&) = default;

	auto& get_underlying_memory() { return *this->allocation.get_memory(); }
	auto& get_underlying_memory() const { return *this->allocation.get_memory(); }
	bool has_private_underlying_memory() const { return allocation.is_private_allocation(); }

	T& get() { return resource; }
	const T& get() const { return resource; }
};

}
}
