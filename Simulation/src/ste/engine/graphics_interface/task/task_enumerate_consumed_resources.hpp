//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <task.hpp>
#include <task_resource.hpp>
#include <task_interface_resource_extractor.hpp>
#include <task_foreach_interface.hpp>

#include <lib/flat_map.hpp>
#include <lib/vector.hpp>

namespace ste {
namespace gl {

namespace _internal {

template <typename Map>
void task_enumerate_consumed_resources_insert(Map &map,
											  const task_resource &resource) {
	auto ret = map.try_emplace(resource.handle, lib::vector<task_resource>{ resource });
	if (!ret.second)
		ret.first->second.push_back(resource);
}

}

/**
 *	@brief	Creates a map of resources consumned by the task interfaces.
 *	
 *	@return	A map of resource handle to a vector of descriptors of access to that resource.
 */
template <class Command, typename task_policy, typename pipeline_policy>
auto task_enumerate_consumed_resources(const _internal::task_impl<Command, task_policy, pipeline_policy> &task) {
	using Task = _internal::task_impl<Command, task_policy, pipeline_policy>;

	lib::flat_map<device_resource_handle, lib::vector<task_resource>> resources_map;

	// Enumerate resources of task interfaces
	_internal::task_foreach_interface<Task>()(&task, [&](auto* interface) {
		using Interface = std::remove_cv_t<std::remove_reference_t<decltype(*interface)>>;

		// Extract resources from interfaces and inject them into the resources map
		for (auto &r : task_interface_extract_consumed_resources<Interface>()(interface))
			_internal::task_enumerate_consumed_resources_insert(resources_map,
																std::move(r));
	});

	return resources_map;
}

}
}
