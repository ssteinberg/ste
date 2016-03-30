
#include "stdafx.hpp"
#include "gpu_task_dispatch_queue.hpp"

#include "AttributedString.hpp"
#include "Log.hpp"

#include <iostream>
#include <algorithm>
#include <functional>

using namespace StE::Graphics;

gpu_task_dispatch_queue::gpu_task_dispatch_queue() : root(std::make_unique<detail::gpu_task_root>()), sop_optimizer(*this) {
	Base::add_vertex(root.get());
}
	
void gpu_task_dispatch_queue::build_task_transitions(const TaskPtr &task) {
	for (auto &t : Base::get_vertices()) {
		if (t==task)
			continue;
			
		if (t->dependencies.find(task) == t->dependencies.end())
			Base::add_edge(gpu_state_transition::transition_function(t, task));
		if (task->dependencies.find(t) == task->dependencies.end())
			Base::add_edge(gpu_state_transition::transition_function(task, t));
	}
}

void gpu_task_dispatch_queue::add_task(const TaskPtr &task, const Core::GenericFramebufferObject *override_fbo, bool mark_inserted) {
	if (mark_inserted)
		task->inserted_into_queue = true;
	task->override_fbo = override_fbo;
	task->parent_queue = this;
	
	Base::erase_all_vertex_edges(task);
	Base::add_vertex(task);

	for (auto &t : task->get_dependencies()) {
		t->requisite_for.insert(task);
		if (Base::get_vertices().find(t) == Base::get_vertices().end())
			add_task(t, nullptr, false);
	}
	
	build_task_transitions(task);
}

void gpu_task_dispatch_queue::add_task(const TaskPtr &task, const Core::GenericFramebufferObject *override_fbo) {
	update_modified_tasks();
	add_task(task, override_fbo, true);
}

void gpu_task_dispatch_queue::remove_task(const TaskPtr &task, bool force) {
	update_modified_tasks();
	
	if (!force && task->requisite_for.size()) {
		using namespace StE::Text::Attributes;

		std::cout << Text::AttributedString("Attempting to remove task from GPU dispatch queue, however task ") + i(task->task_name()) + " is a requisite for " + i((*task->requisite_for.begin())->task_name()) + "." << std::endl;
		ste_log_fatal() << "task " << task->task_name() << " is a requisite for " << (*task->requisite_for.begin())->task_name() << std::endl;
		assert(false && "Task is a requisite!");
		return;
	}
	
	task->inserted_into_queue = false;
	task->override_fbo = nullptr;
	
	Base::erase_vertex(task);
	
	for (auto &t : task->dependencies) {
		t->requisite_for.erase(task);
		if (!t->inserted_into_queue && !t->requisite_for.size())
			remove_task(t);
	}
	
	auto it = std::find(modified_tasks.begin(), modified_tasks.end(), task);
	if (it != modified_tasks.end())
		modified_tasks.erase(it);
}

void gpu_task_dispatch_queue::update_task_fbo(const TaskPtr &task, const Core::GenericFramebufferObject *override_fbo) const {
	task->set_override_fbo(override_fbo);
}

void gpu_task_dispatch_queue::remove_all() {
	decltype(Base::get_vertices().begin()) it;
	while ((it = Base::get_vertices().begin()) != Base::get_vertices().end())
		remove_task(*it, true);
}

void gpu_task_dispatch_queue::update_modified_tasks() {
	if (modified_tasks.size()) {
		for (auto &ptr : modified_tasks) {
			auto fbo = ptr->get_override_fbo();
			add_task(ptr, fbo, false);
		}
		modified_tasks.clear();
	}
}

void gpu_task_dispatch_queue::dispatch() {
	update_modified_tasks();
	
	auto solution = this->sop_optimizer(root.get());
	for (auto &p : solution.route)
		p.second->dispatch();
}
