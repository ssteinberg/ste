//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_queue_type.hpp>

namespace ste {
namespace gl {

class device_pipeline_graphics;
class device_pipeline_compute;

template <ste_queue_type QueueType>
struct task_pipeline_policy {
	static constexpr ste_queue_type queue_type = QueueType;
};

template <>
struct task_pipeline_policy<ste_queue_type::graphics_queue> {
	static constexpr ste_queue_type queue_type = ste_queue_type::graphics_queue;
	using pipeline_object_type = device_pipeline_graphics;
};

template <>
struct task_pipeline_policy<ste_queue_type::compute_queue> {
	static constexpr ste_queue_type queue_type = ste_queue_type::compute_queue;
	using pipeline_object_type = device_pipeline_compute;
};

}
}
