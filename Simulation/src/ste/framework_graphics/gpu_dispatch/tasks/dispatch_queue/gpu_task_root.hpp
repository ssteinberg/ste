// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_task.hpp"

namespace StE {
namespace Graphics {
namespace detail {

class gpu_task_root : public gpu_task {
protected:
	void dispatch() const override final {}
};

}
}
}
