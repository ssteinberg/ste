// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "gpu_task.h"

#include <memory>
#include <unordered_set>

namespace StE {
namespace Graphics {

class gpu_task_dispatch_queue {
private:
	std::unordered_set<std::shared_ptr<gpu_task>> tasks;
	
	void dispatch(const std::shared_ptr<gpu_task> &task, std::unordered_set<std::shared_ptr<gpu_task>> &tasks_to_dispatch) const {
		task->update();
		
		for (auto &d : task->dependencies)
			dispatch(d, tasks_to_dispatch);
			
		(*task)();
		
		tasks_to_dispatch.erase(task);
	}

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

	void dispatch() const {
		auto tasks_to_dispatch = tasks;
		while ((auto it = tasks_to_dispatch.begin()) != tasks_to_dispatch.end()) 
			dispatch(*it, tasks_to_dispatch);
	}
};

}
}
