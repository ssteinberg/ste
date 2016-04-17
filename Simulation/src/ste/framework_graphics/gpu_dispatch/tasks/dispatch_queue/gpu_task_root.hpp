// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_dummy_dispatchable.hpp"
#include "gpu_task.hpp"

namespace StE {
namespace Graphics {
namespace detail {

class gpu_task_root : public gpu_task {
	gpu_dummy_dispatchable d;

public:
	gpu_task_root() : gpu_task(gpu_task::AccessToken(), "root", &d) {}
};

}
}
}
