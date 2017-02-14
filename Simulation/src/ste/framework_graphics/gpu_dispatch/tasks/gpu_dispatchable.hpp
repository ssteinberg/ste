// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <gl_context_state_log.hpp>

namespace StE {
namespace Graphics {

class gpu_task;
class gpu_state_transition;

class gpu_dispatchable {
	friend class gpu_task;
	friend class gpu_state_transition;

private:
	mutable Core::GL::gl_context_state_log context_state_descriptor;

protected:
	// Should set up gl resources, states, etc..
	virtual void set_context_state() const = 0;
	// Should update buffers, locks, etc. and call a single render/compute method.
	virtual void dispatch() const = 0;

public:
	virtual ~gpu_dispatchable() noexcept {}
};

}
}
