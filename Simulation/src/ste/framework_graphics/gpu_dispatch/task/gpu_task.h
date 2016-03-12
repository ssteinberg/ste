// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "gpu_dispatch.h"
#include "gpu_state.h"

#include <set>
#include <memory>

namespace StE {
namespace Graphics {
	
class gpu_task_dispatch_queue;

class gpu_task {
private:
	friend class gpu_task_dispatch_queue;

private:
	std::set<std::shared_ptr<gpu_task>> dependencies;
	std::set<std::shared_ptr<gpu_task>> requisite_for;
	bool inserted_into_queue{ false };

protected:
	void add_dependency(const std::shared_ptr<gpu_task> &task) {
		dependencies.insert(task);
	}
	void remove_dependency(const std::shared_ptr<gpu_task> &task) {
		dependencies.erase(task);
	}

public:
	virtual ~gpu_task() noexcept {}
	
	void operator()() const {
		for (auto &d : dependencies)
			d();
		dispatch_state()();
		dispatcher->dispatch();
	}
	
public:
	// Defines dispatch state
	virtual gpu_state dispatch_state() const = 0;
	
protected:
	// Should update buffers, uniforms, etc.. Called before dispatching any dependencies.
	virtual void update() const = 0;
	// Should set locks/sync objects, etc. and call a glDraw*/glDispatch* method only.
	virtual void dispatch() const = 0;
};

}
}
