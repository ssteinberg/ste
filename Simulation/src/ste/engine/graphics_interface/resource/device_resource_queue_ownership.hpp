//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_device_queue.hpp>

#include <atomic>

namespace StE {
namespace GL {

struct device_resource_queue_ownership {
	using queue_index_t = ste_device_queue::queue_index_t;
	using resource_queue_selector_t = const ste_queue_selector<ste_queue_selector_default_policy> &;

	std::atomic<queue_index_t> index;

	device_resource_queue_ownership() = delete;
	device_resource_queue_ownership(const ste_context &ctx,
									const resource_queue_selector_t &selector)
		: index(ctx.device().select_queue(selector)->index())
	{}
	device_resource_queue_ownership(const queue_index_t &index)
		: index(index)
	{}

	device_resource_queue_ownership(device_resource_queue_ownership &&o) noexcept
		: index(o.index.load()) {}
	device_resource_queue_ownership &operator=(device_resource_queue_ownership &&o) noexcept {
		index.store(o.index);
		return *this;
	}
};

}
}
