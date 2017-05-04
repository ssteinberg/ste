//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <task.hpp>
#include <task_resource.hpp>
#include <task_interface_resource_extractor.hpp>
#include <task_pipeline_resource_extractor.hpp>
#include <task_foreach_interface.hpp>

#include <unordered_map>
#include <vector>

namespace ste {
namespace gl {

namespace _internal {

template <typename Map>
void task_enumerate_consumed_resources_insert(Map &map,
											  const task_resource &resource) {
	auto ret = map.try_emplace(resource.handle, std::vector<task_resource>{ resource });
	if (!ret.second)
		ret.first->second.push_back(resource);
}

template <typename pipeline_policy, typename = std::void_t<>>
struct task_enumerate_consumed_resources_extract_pipeline_resources {
	template <typename Task, typename Map>
	void operator()(const Task *task,
					Map &map) {}
};
template <typename pipeline_policy>
struct task_enumerate_consumed_resources_extract_pipeline_resources<pipeline_policy, std::void_t<typename pipeline_policy::pipeline_object_type>>  {
	template <typename Task, typename Map>
	void operator()(const Task *task,
					Map &map) {
		for (auto &r : task_pipeline_policy_extract_pipeline_resources<typename pipeline_policy::pipeline_object_type>()(task))
			_internal::task_enumerate_consumed_resources_insert(map,
																std::move(r));
	}
};

}

template <class Command, typename task_policy, typename pipeline_policy>
auto task_enumerate_consumed_resources(const _internal::task_impl<Command, task_policy, pipeline_policy> &task) {
	using Task = _internal::task_impl<Command, task_policy, pipeline_policy>;

	std::unordered_map<device_resource_handle, std::vector<task_resource>> resources_map;

	// Enumerate resources of task interfaces
	_internal::task_foreach_interface<Task>()(&task, [&](auto* interface) {
		using Interface = std::remove_cv_t<std::remove_reference_t<decltype(*interface)>>;

		// Extract resources from interfaces and inject them into the resources map
		for (auto &r : task_interface_extract_consumed_resources<Interface>()(interface))
			_internal::task_enumerate_consumed_resources_insert(resources_map,
																std::move(r));
	});

	// Enumerate pipeline resources
	_internal::task_enumerate_consumed_resources_extract_pipeline_resources<pipeline_policy>()(&task, 
																							   resources_map);

	return resources_map;
}

}
}
