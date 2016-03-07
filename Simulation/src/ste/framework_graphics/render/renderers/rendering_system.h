// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "rendering_queue.h"

#include <string>

namespace StE {

class StEngineControl;

namespace Graphics {

class rendering_system {
private:
	rendering_queue q;

public:
	virtual ~rendering_system() noexcept {}

	virtual void finalize_queue(const StEngineControl &ctx) = 0;
	virtual void render_queue(const StEngineControl &ctx) = 0;

	rendering_queue& queue() { return q; };

	virtual std::string rendering_system_name() const = 0;
};

}
}
