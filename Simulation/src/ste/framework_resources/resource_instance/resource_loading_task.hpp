// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "task_future.hpp"
#include "optional.hpp"

namespace StE {
namespace Resource {

template <typename R>
class resource_loading_task {
private:
	using resource_instance_future = task_future<void>;

public:
	optional<resource_instance_future> loader(const StEngineControl &ctx, R* res) {
		return none;
	}
};

}
}
