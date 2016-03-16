
#include "stdafx.h"
#include "gpu_task_dispatch_queue.h"

#include <iostream>

using namespace StE::Graphics;

void gpu_task_dispatch_queue::dispatch(TaskPtr task, TasksCollection &tasks_to_dispatch) const {		
	tasks_to_dispatch.erase(task);
	
	for (auto &d : task->dependencies) {
		if (tasks_to_dispatch.find(d) != tasks_to_dispatch.end())
			dispatch(d, tasks_to_dispatch);
	}

	(*task)();
}

void gpu_task_dispatch_queue::add_task(const TaskPtr &task, const LLR::GenericFramebufferObject *override_fbo) {
	task->inserted_into_queue = true;
	update_task_fbo(task, override_fbo);
	
	tasks.insert(task);
	for (auto &t : task->dependencies) {
		tasks.insert(t);
		t->requisite_for.insert(task);
	}
	
	for (auto &sub_t : task->sub_tasks) {
		if (tasks.find(sub_t) == tasks.end()) {
			task->dependencies.insert(sub_t);
			sub_t->dependencies.insert(task->dependencies.begin(), task->dependencies.end());
			add_task(sub_t);
		}
	}
}

void gpu_task_dispatch_queue::remove_task(const TaskPtr &task) {
	if (task->requisite_for.size()) {
		assert(false && "Task is a prerequisite!");
		return;
	}
	
	task->inserted_into_queue = false;
	update_task_fbo(task, nullptr);
	
	tasks.erase(task);
	for (auto &t : task->dependencies) {
		t->requisite_for.erase(task);
		if (!t->inserted_into_queue && !t->requisite_for.size())
			tasks.erase(t);
	}
}

void gpu_task_dispatch_queue::update_task_fbo(const TaskPtr &task, const LLR::GenericFramebufferObject *override_fbo) const {
	task->override_fbo = override_fbo;
}

void gpu_task_dispatch_queue::dispatch() const {
	auto tasks_to_dispatch = tasks;
	
	decltype(tasks_to_dispatch.begin()) it;
	while ((it = tasks_to_dispatch.begin()) != tasks_to_dispatch.end()) 
		dispatch(*it, tasks_to_dispatch);
}
