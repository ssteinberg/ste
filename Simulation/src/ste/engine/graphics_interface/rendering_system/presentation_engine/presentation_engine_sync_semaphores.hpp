//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste_device_sync_primitives_pools.hpp>

namespace ste {
namespace gl {

struct presentation_engine_sync_semaphores {
	using semaphore_t = ste_device_sync_primitives_pools::semaphore_pool_t::resource_t;
	semaphore_t swapchain_image_ready_semaphore;
	semaphore_t rendering_finished_semaphore;

	presentation_engine_sync_semaphores() = delete;
	presentation_engine_sync_semaphores(semaphore_t &&s1, semaphore_t &&s2)
		: swapchain_image_ready_semaphore(std::move(s1)),
		rendering_finished_semaphore(std::move(s2))
	{}
};

}
}
