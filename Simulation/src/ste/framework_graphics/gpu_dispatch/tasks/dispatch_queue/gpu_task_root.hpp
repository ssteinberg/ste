// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_task.hpp"

namespace StE {
namespace Graphics {
namespace detail {

class gpu_task_root_dispatchable : public gpu_dispatchable {
protected:
	void set_context_state() const override final {}
	void dispatch() const override final {}
};

class gpu_task_root : public gpu_task {
	gpu_task_root_dispatchable d;

public:
	gpu_task_root() : gpu_task(gpu_task::AccessToken(), "root", &d) {}
};

}
}
}
