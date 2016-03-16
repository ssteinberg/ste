// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "gpu_task.h"

#include "FramebufferObject.h"

#include <memory>
#include <unordered_set>

namespace StE {
namespace Graphics {

class gpu_task_dispatch_queue {
private:
	using TaskT = typename gpu_task::TaskT;
	using TaskPtr = typename gpu_task::TaskPtr;
	using TasksCollection = typename gpu_task::TasksCollection;

private:
	TasksCollection tasks;
	
	void dispatch(TaskPtr task, TasksCollection &tasks_to_dispatch) const;

public:
	void add_task(const TaskPtr &task, const LLR::GenericFramebufferObject *override_fbo = nullptr);	
	void remove_task(const TaskPtr &task);
	void update_task_fbo(const TaskPtr &task, const LLR::GenericFramebufferObject *override_fbo) const;

	void dispatch() const;
};

}
}
