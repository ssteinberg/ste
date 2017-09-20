//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_device_sync_primitives_pools.hpp>
#include <pipeline_stage.hpp>

namespace ste {
namespace gl {

struct wait_semaphore {
	using semaphore_t = ste_device_sync_primitives_pools::semaphore_pool_t::resource_t;

	semaphore_t sem;
	pipeline_stage stage{ pipeline_stage::top_of_pipe };

	wait_semaphore(semaphore_t &&sem, pipeline_stage stage)
		: sem(std::move(sem)),
		  stage(stage) {}

	operator std::pair<VkSemaphore, VkPipelineStageFlags>() const {
		return std::make_pair(static_cast<VkSemaphore>(sem.get()), static_cast<VkPipelineStageFlags>(stage));
	}

	operator VkSemaphore() const {
		return static_cast<VkSemaphore>(sem.get());
	}
};

}
}
