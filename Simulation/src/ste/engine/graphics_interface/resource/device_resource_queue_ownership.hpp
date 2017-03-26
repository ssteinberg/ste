//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_queue_family.hpp>

#include <atomic>

namespace StE {
namespace GL {

struct device_resource_queue_ownership {
	using family_t = ste_queue_family;

	static_assert(std::is_pod_v<family_t>, "family_t should be a POD");

	std::atomic<family_t> family;

	device_resource_queue_ownership() = delete;
	template <typename selector_policy>
	device_resource_queue_ownership(const ste_context &ctx,
									const ste_queue_selector<selector_policy> &selector)
		: family(ctx.device().select_queue(selector)->queue_descriptor().family)
	{
		assert(this->family.is_lock_free());
	}
	device_resource_queue_ownership(const family_t &family)
		: family(family)
	{
		assert(this->family.is_lock_free());
	}

	device_resource_queue_ownership(device_resource_queue_ownership &&o) noexcept
		: family(o.family.load()) {}
	device_resource_queue_ownership &operator=(device_resource_queue_ownership &&o) noexcept {
		family.store(o.family);
		return *this;
	}
};

}
}
