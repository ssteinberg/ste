//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <task_resource.hpp>

#include <device_pipeline_graphics.hpp>
#include <device_pipeline_compute.hpp>

namespace ste {
namespace gl {

namespace _internal {

template <typename Pipeline>
void task_pipeline_policy_extract_pipeline_bound_resources(const Pipeline &pipeline, 
														   std::vector<task_resource> &ret) {
	const auto& bound_resources = pipeline.get_bound_resources_map();
	for (const pipeline_consumed_resource &r : bound_resources) {

	}
}

}

template <class pipeline_object_type>
struct task_pipeline_policy_extract_pipeline_resources {};

template <>
struct task_pipeline_policy_extract_pipeline_resources<device_pipeline_graphics> {
	template <typename Task>
	auto operator()(const Task *task) {
		std::vector<task_resource> ret;

		const device_pipeline_graphics &pipeline = *task->get_pipeline_storage(Task::accessor_token()).obj;
		_internal::task_pipeline_policy_extract_pipeline_bound_resources(pipeline, ret);

		return ret;
	}
};

template <>
struct task_pipeline_policy_extract_pipeline_resources<device_pipeline_compute> {
	template <typename Task>
	auto operator()(const Task *task) {
		std::vector<task_resource> ret;

		const device_pipeline_compute &pipeline = *task->get_pipeline_storage(Task::accessor_token()).obj;
		_internal::task_pipeline_policy_extract_pipeline_bound_resources(pipeline, ret);

		return ret;
	}
};

}
}
