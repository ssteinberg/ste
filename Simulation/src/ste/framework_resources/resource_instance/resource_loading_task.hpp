// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "ste_engine_control.hpp"

#include "task_future.hpp"
#include "optional.hpp"

namespace StE {
namespace Resource {

/**
*	@brief	Defines loader for resource_instance<R>
*
*	Partial specializations can be defined to specify a loading task. loader() takes a context reference and a pointer
*	to the resource being loaded, and returns a task_future. Default implementations does nothing.
*
 *	@param R	resource type
*/
template <typename R>
class resource_loading_task {
private:
	using resource_instance_future = task_future<void>;

public:
	optional<resource_instance_future> loader(const ste_engine_control &ctx, R* res) {
		return none;
	}
};

}
}
