
#include "stdafx.hpp"
#include "gpu_task.hpp"

#include "gpu_task_dispatch_queue.hpp"

using namespace StE::Graphics;

void gpu_task::add_dependency(const TaskPtr &task) const {
	assert(task.get() != this);
	
	if (task.get() != this) {
		if (parent_queue) {
			parent_queue->add_task_dependency(shared_from_this(), task);
			return;
		}
		
		dependencies.insert(task);
		for (auto &s : sub_tasks)
			s->add_dependency(task);
	}
}
void gpu_task::remove_dependency(const TaskPtr &task) const {
	if (parent_queue) {
		parent_queue->remove_task_dependency(shared_from_this(), task);
		return;
	}
	
	dependencies.erase(task);
	for (auto &s : sub_tasks)
		s->remove_dependency(task);
}
