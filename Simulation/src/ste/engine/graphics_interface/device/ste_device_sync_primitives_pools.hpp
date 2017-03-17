//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vk_logical_device.hpp>

#include <unique_fence.hpp>
#include <shared_fence.hpp>
#include <vk_event.hpp>
#include <vk_semaphore.hpp>
#include <ste_resource_pool.hpp>

namespace StE {
namespace GL {

class ste_device_sync_primitives_pools {
public:
	using unique_fence_pool_t = ste_resource_pool<unique_fence<void>>;
	using shared_fence_pool_t = ste_resource_pool<shared_fence<void>>;
	using event_pool_t = ste_resource_pool<vk_event>;
	using semaphore_pool_t = ste_resource_pool<vk_semaphore>;

private:
	unique_fence_pool_t fence_pool;
	shared_fence_pool_t shared_fence_pool;
	event_pool_t event_pool;
	semaphore_pool_t semaphore_pool;

public:
	ste_device_sync_primitives_pools(const vk_logical_device &device)
		: fence_pool(device),
		shared_fence_pool(device),
		event_pool(device),
		semaphore_pool(device)
	{}

	auto& unique_fences() { return fence_pool; }
	auto& shared_fences() { return shared_fence_pool; }
	auto& events() { return event_pool; }
	auto& semaphores() { return semaphore_pool; }
};

}
}
