//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <task.hpp>
#include <task_resource.hpp>

#include <boost/container/flat_map.hpp>

namespace ste {
namespace gl {

template <class Command, typename task_policy, typename pipeline_policy>
auto task_enumerate_consumed_resources(const _internal::task_impl<Command, task_policy, pipeline_policy> &task) {
	boost::container::flat_map<task_resource_handle, task_resource> resources_map;

	return resources_map;
}

}
}
