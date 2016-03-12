// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "gpu_task.h"

#include <memory>
#include <vector>

namespace StE {
namespace Graphics {

class gpu_task_dispatch_queue {
private:
	std::set<std::shared_ptr<gpu_task>> tasks;

public:
	void add_task(const std::shared_ptr<gpu_task> &task) {
		tasks.insert(task);
		task->inserted_into_queue = true;
		for (auto &t : task->dependencies) {
			tasks.insert(t);
			t->requisite_for.insert(task);
		}
	}
	
	void remove_task(const std::shared_ptr<gpu_task> &task) {
		if (task->requisite_for.count()) {
			assert(false && "Task is a prerequisite!");
			return;
		}
		
		task->inserted_into_queue = false;
		tasks.erase(task);
		for (auto &t : task->dependencies) {
			t->requisite_for.erase(task);
			if (!t->inserted_into_queue && !t->requisite_for.count())
				tasks.erase(t);
		}
	}

	void dispatch() {
		auto tasks_to_dispatch = tasks;
		while ((auto it = tasks_to_dispatch.begin()) != tasks_to_dispatch.end()) {
			for (auto &d : it->dependencies) {
				d();
				tasks_to_dispatch.erase(d);
			}
			(*it)();
			tasks_to_dispatch.erase(it);
		}
	}
};

}
}
