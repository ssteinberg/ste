// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "gpu_task_dispatch_queue.hpp"

#include <string>

namespace StE {

class StEngineControl;

namespace Graphics {

class rendering_system {
protected:
	gpu_task_dispatch_queue q;

public:
	virtual ~rendering_system() noexcept {}

	virtual void render_queue(const StEngineControl &ctx) = 0;

	virtual std::string rendering_system_name() const = 0;
};

}
}
