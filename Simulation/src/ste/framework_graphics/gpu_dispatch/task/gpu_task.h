// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "gpu_dispatch.h"

#include <unordered_set>
#include <memory>

namespace StE {
namespace Graphics {

class gpu_task_dispatch_queue;

class gpu_task {
private:
	friend class gpu_task_dispatch_queue;

private:
	std::unordered_set<std::shared_ptr<gpu_task>> dependencies;
	std::unordered_set<std::shared_ptr<gpu_task>> requisite_for;
	bool inserted_into_queue{ false };
	
protected:
	const auto &get_dependencies() const { return dependencies; }

protected:
	void add_dependency(const std::shared_ptr<gpu_task> &task) {
		dependencies.insert(task);
	}
	void remove_dependency(const std::shared_ptr<gpu_task> &task) {
		dependencies.erase(task);
	}
	
	void operator()() const {
		set_context_state();
		dispatch();
	}

public:
	virtual ~gpu_task() noexcept {}
	
protected:
	// Should set up gl resources, states, etc..
	virtual void set_context_state() const {};
	// Should update buffers, locks, etc. and call a single render/compute method.
	virtual void dispatch() const = 0;
};

}
}
