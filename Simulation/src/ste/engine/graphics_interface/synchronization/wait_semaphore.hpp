//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <semaphore.hpp>
#include <pipeline_stage.hpp>

namespace ste {
namespace gl {

struct wait_semaphore {
	const semaphore *sem{ nullptr };
	pipeline_stage stage{ pipeline_stage::top_of_pipe };

	wait_semaphore() = default;
	wait_semaphore(const semaphore *sem, pipeline_stage stage) : sem(sem), stage(stage) {}

	operator std::pair<VkSemaphore, VkPipelineStageFlags>() const {
		return std::make_pair(static_cast<VkSemaphore>(*sem), static_cast<VkPipelineStageFlags>(stage));
	}
};

}
}
