
#include "stdafx.h"
#include "gpu_task_dispatch_queue.h"

#include "AttributedString.h"
#include "Log.h"

#include <iostream>

using namespace StE::Graphics;

void gpu_task_dispatch_queue::dispatch(TaskPtr task, TasksCollection &tasks_to_dispatch) const {
	tasks_to_dispatch.erase(task);
	
	for (auto &d : task->after) {
		if (tasks_to_dispatch.find(d) != tasks_to_dispatch.end())
			dispatch(d, tasks_to_dispatch);
	}
	
	for (auto &d : task->dependencies) {
		if (tasks_to_dispatch.find(d) != tasks_to_dispatch.end())
			dispatch(d, tasks_to_dispatch);
	}

	(*task)();
}

void gpu_task_dispatch_queue::add_task(const TaskPtr &task, const LLR::GenericFramebufferObject *override_fbo, bool mark_inserted) {
	if (mark_inserted)
		task->inserted_into_queue = true;
	task->override_fbo = override_fbo;
	task->parent_queue = this;
	task->dependencies = task->task_dependencies;
	
	tasks.insert(task);
	
	for (auto &sub_t : task->sub_tasks) {
		add_task(sub_t, override_fbo, false);
		sub_t->dependencies.insert(task->task_dependencies.begin(), task->task_dependencies.end());
		task->dependencies.insert(sub_t);
	}
	
	for (auto &t : task->dependencies) {
		tasks.insert(t);
		t->requisite_for.insert(task);
	}
}

void gpu_task_dispatch_queue::add_task(const TaskPtr &task, const LLR::GenericFramebufferObject *override_fbo) {
	add_task(task, override_fbo, true);
}

void gpu_task_dispatch_queue::remove_task(const TaskPtr &task, bool force) {
	if (!force && task->requisite_for.size()) {
		using namespace StE::Text::Attributes;
			
		std::cout << Text::AttributedString("Attempting to remove task from GPU dispatch queue, however task ") + i(task->task_name()) + " is a requisite for " + i((*task->requisite_for.begin())->task_name()) + "." << std::endl;
		ste_log_fatal() << "task " << task->task_name() << " is a requisite for " << (*task->requisite_for.begin())->task_name() << std::endl;
		assert(false && "Task is a requisite!");
		return;
	}
	
	task->inserted_into_queue = false;
	task->override_fbo = nullptr;
	task->parent_queue = nullptr;
	tasks.erase(task);
	
	for (auto &t : task->dependencies) {
		t->requisite_for.erase(task);
		if (!t->inserted_into_queue && !t->requisite_for.size())
			remove_task(t);
	}

	task->dependencies.clear();
	modified_tasks.erase(task);
}

void gpu_task_dispatch_queue::update_task_fbo(const TaskPtr &task, const LLR::GenericFramebufferObject *override_fbo) const {
	task->set_override_fbo(override_fbo);
}

void gpu_task_dispatch_queue::remove_all() {
	decltype(tasks.begin()) it;
	while ((it = tasks.begin()) != tasks.end())
		remove_task(*it, true);
}

void gpu_task_dispatch_queue::dispatch() {
	if (modified_tasks.size()) {
		for (auto &ptr : modified_tasks) 
			add_task(ptr, ptr->get_override_fbo());
		modified_tasks.clear();
	}
	
	auto tasks_to_dispatch = tasks;
	
	decltype(tasks_to_dispatch.begin()) it;
	while ((it = tasks_to_dispatch.begin()) != tasks_to_dispatch.end()) 
		dispatch(*it, tasks_to_dispatch);
}
