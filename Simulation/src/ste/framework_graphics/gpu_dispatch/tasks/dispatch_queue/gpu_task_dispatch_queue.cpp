
#include "stdafx.hpp"
#include "gpu_task_dispatch_queue.hpp"

#include "AttributedString.hpp"
#include "Log.hpp"

#include "tuple_call.hpp"

#include <iostream>
#include <algorithm>
#include <functional>
#include <iostream>

#include <chrono>
#include <immintrin.h>

using namespace StE::Graphics;

gpu_task_dispatch_queue::gpu_task_dispatch_queue() : root(std::make_shared<detail::gpu_task_root>()),
													 sop(*this) {
	Base::add_vertex(root);
}

gpu_task_dispatch_queue::~gpu_task_dispatch_queue() noexcept {}

void gpu_task_dispatch_queue::build_task_transitions(const TaskPtr &task) {
	for (auto &t : Base::get_vertices()) {
		if (t==task)
			continue;

		if (t->dependencies.find(task) == t->dependencies.end())
			Base::add_edge(std::make_unique<gpu_state_transition>(t.get(), task.get()));
		if (task->dependencies.find(t) == task->dependencies.end())
			Base::add_edge(std::make_unique<gpu_state_transition>(task.get(), t.get()));
	}
}

void gpu_task_dispatch_queue::insert_task(const std::shared_ptr<TaskT> &task, bool mark_inserted) {
	if (mark_inserted)
		task->inserted_into_queue = true;
	task->set_parent_queue(this);

	Base::erase_all_vertex_edges(task.get());
	Base::add_vertex(task);
	build_task_transitions(task);

	bool dep_inserted = false;
	for (auto &t : task->get_dependencies()) {
		assert(t != task);

		if (Base::get_vertices().find(t) == Base::get_vertices().end()) {
			dep_inserted = true;
			insert_task(t, false);
		}
		t->requisite_for.insert(task.get());
	}

	if (!dep_inserted)
		sop.clear_best_solution();
}

void gpu_task_dispatch_queue::erase_task(const TaskPtr &task, bool force) {
	if (!force && task->requisite_for.size()) {
		using namespace StE::Text::Attributes;

		std::cout << Text::AttributedString("Attempting to remove task from GPU dispatch queue, however task ") + i(task->get_name()) + " is a requisite for " + i((*task->requisite_for.begin())->get_name()) + "." << std::endl;
		ste_log_fatal() << "task " << task->get_name() << " is a requisite for " << (*task->requisite_for.begin())->get_name() << std::endl;
		assert(false && "Task is a requisite!");
		return;
	}
	else if (force)
		task->requisite_for.clear();

	task->inserted_into_queue = false;
	task->set_parent_queue(nullptr);

	Base::erase_vertex(task);

	bool dep_erased = false;
	for (auto &t : task->dependencies) {
		t->requisite_for.erase(task.get());
		if (!t->inserted_into_queue && !t->requisite_for.size()) {
			dep_erased = true;
			erase_task(t, false);
		}
	}

	if (!dep_erased)
		sop.clear_best_solution();
}

void gpu_task_dispatch_queue::erase_all() {
	decltype(Base::get_vertices().begin()) it;
	while ((it = Base::get_vertices().begin()) != Base::get_vertices().end())
		erase_task(*it, true);
}

void gpu_task_dispatch_queue::add_dep(const TaskPtr &task, const TaskPtr &dep) {
	task->dependencies.insert(dep);

	for (auto &s : task->sub_tasks)
		add_dep(s, dep);

	if (Base::get_vertices().find(dep) == Base::get_vertices().end()) {
		insert_task(dep, false);
	}
	else {
		Base::erase_edge(task.get(), dep.get());
		sop.clear_best_solution();
	}

	dep->requisite_for.insert(task.get());
}

void gpu_task_dispatch_queue::delete_dep(const TaskPtr &task, const TaskPtr &dep) {
	task->dependencies.erase(dep);
	dep->requisite_for.erase(task.get());

	for (auto &s : task->sub_tasks)
		delete_dep(s, dep);

	if (Base::get_vertices().find(task) != Base::get_vertices().end() &&
		Base::get_vertices().find(dep) != Base::get_vertices().end())
		Base::add_edge(std::make_unique<gpu_state_transition>(task.get(), dep.get()));
}

void gpu_task_dispatch_queue::run_sop_iteration() {
	// auto start_time = std::chrono::high_resolution_clock::now();
	// auto rdts_stamp = _rdtsc();
	sop.optimizer(root.get(), 1)();
	// unsigned cycles = _rdtsc() - rdts_stamp;
	// unsigned ms = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
}

void gpu_task_dispatch_queue::dispatch_sop_solution() {
	bool ret;

	if (prof != nullptr) {
		ret = sop.run_solution([this](const gpu_state_transition *e){
								   const gpu_task *task = reinterpret_cast<const gpu_task*>(e->get_to());
								   task->query_start();
							   }, [this](const gpu_state_transition *e){
								   const gpu_task *task = reinterpret_cast<const gpu_task*>(e->get_to());
								   task->query_end();

								   profiler_entry entry;
								   entry.name = task->get_name();
								   entry.start = task->get_start_time();
								   entry.end = task->get_end_time();
								   this->prof->add_entry(entry);
							   });
	}
	else {
		ret = sop.run_solution();
	}

	if (!ret) {
		assert(false);
		ste_log_error() << "GPU dispatch: No SOP solution available during dispatch!" << std::endl;
		return;
	}
}

void gpu_task_dispatch_queue::update_modified_tasks() {
	std::unique_ptr<modify_task_log> l;
	while ((l = modify_queue.pop()) != nullptr)
		(*l)();
}

void gpu_task_dispatch_queue::dispatch() {
	update_modified_tasks();
	run_sop_iteration();
	dispatch_sop_solution();
}
