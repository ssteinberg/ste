// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_task.hpp"
#include "gpu_state_transition.hpp"

#include "FramebufferObject.hpp"

#include <memory>
#include <unordered_set>
#include <vector>

#include "graph.hpp"

namespace StE {
namespace Graphics {

class gpu_task_dispatch_queue {
	friend class gpu_task;
	
private:
	using TaskT = typename gpu_task::TaskT;
	using TaskPtr = typename gpu_task::TaskPtr;
	using TasksCollection = typename gpu_task::TasksCollection;

private:
	TasksCollection tasks, modified_tasks;
	
	void add_task(const TaskPtr&, const Core::GenericFramebufferObject*, bool);
	
	void dispatch(TaskPtr, TasksCollection&) const;
	
protected:
	void signal_task_modified(const TaskPtr &task) { modified_tasks.insert(task); }

public:
	void add_task(const TaskPtr &task, const Core::GenericFramebufferObject *override_fbo = nullptr);
	void remove_task(const TaskPtr &task, bool force= false);
	void update_task_fbo(const TaskPtr &task, const Core::GenericFramebufferObject *override_fbo) const;
	
	void remove_all();

	void dispatch();
	
	Graph::graph<gpu_task, gpu_state_transition> create_transition_graph() const;
};

}
}
