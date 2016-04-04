
#include "stdafx.hpp"
#include "gpu_task_dispatch_queue.hpp"

#include "AttributedString.hpp"
#include "Log.hpp"

#include "thread_priority.hpp"

#include "tuple_call.hpp"

#include <iostream>
#include <algorithm>
#include <functional>

#include <iostream>
#include <thread>

using namespace StE::Graphics;

gpu_task_dispatch_queue::gpu_task_dispatch_queue() : root(std::make_shared<detail::gpu_task_root>()),
													 sop(*this),
													 optimizer_thread([this]() {
														auto flag = interruptible_thread::interruption_flag;

														for (;;) {
															if (flag->is_set()) return;

															if (sop.get_no_improvements_counter() >= max_optimizer_runs_without_improvement)
																ste_log() << "SOP optimization ran for " << max_optimizer_runs_without_improvement << " iterations without improvement. Stopping." << std::endl;

															std::unique_lock<std::mutex> l(this->optimizer_notification_mutex);
															this->optimizer_notifier.wait(l, [&]() {
																return flag->is_set() || sop.get_no_improvements_counter() < max_optimizer_runs_without_improvement;
															});

															if (flag->is_set()) return;

															std::unique_lock<std::mutex> l2(this->optimizer_mutex);
															auto optimizer_guard = current_optimizer.emplace_and_acquire(std::memory_order_release, this->sop, root.get(), optimizer_iterations);
															(*optimizer_guard)();
														}
													}) {
	Base::add_vertex(root);

	thread_set_priority_low(&optimizer_thread.get_thread());
}

gpu_task_dispatch_queue::~gpu_task_dispatch_queue() noexcept {
	optimizer_thread.interrupt();
	optimizer_notifier.notify_one();
	auto mg = interrupt_optimizer_and_acquire_mutex();

	mg.unlock();
	if (optimizer_thread.joinable())
		optimizer_thread.join();
}

std::unique_lock<std::mutex> gpu_task_dispatch_queue::interrupt_optimizer_and_acquire_mutex() {
	std::unique_lock<std::mutex> ul(optimizer_mutex, std::defer_lock);

	while (!ul.try_lock()) {
		auto optimizer_guard = current_optimizer.acquire(std::memory_order_acquire);
		if (optimizer_guard.is_valid())
			optimizer_guard->notify_stop();
		std::this_thread::yield();
	}

	return ul;
}

void gpu_task_dispatch_queue::clear_sop_solution_unlock_and_notify(std::unique_lock<std::mutex> &&ul) {
	sop.clear_best_solution();
	// Generate a new solution
	sop.optimizer(root.get(), 1)();

	ul.unlock();
	optimizer_notifier.notify_one();
}

void gpu_task_dispatch_queue::build_task_transitions(const TaskPtr &task) {
	for (auto &t : Base::get_vertices()) {
		if (t==task)
			continue;

		if (t->dependencies.find(task) == t->dependencies.end())
			Base::add_edge(gpu_state_transition::transition_function(t.get(), task.get()));
		if (task->dependencies.find(t) == task->dependencies.end())
			Base::add_edge(gpu_state_transition::transition_function(task.get(), t.get()));
	}
}

void gpu_task_dispatch_queue::insert_task(const std::shared_ptr<TaskT> &task, const Core::GenericFramebufferObject *override_fbo, bool mark_inserted) {
	if (mark_inserted)
		task->inserted_into_queue = true;
	task->override_fbo = override_fbo;
	task->set_parent_queue(this);

	Base::erase_all_vertex_edges(task.get());
	Base::add_vertex(task);

	build_task_transitions(task);

	for (auto &t : task->get_dependencies()) {
		assert(t != task);

		if (Base::get_vertices().find(t) == Base::get_vertices().end())
			insert_task(t, nullptr, false);
		t->requisite_for.insert(task.get());
	}
}

void gpu_task_dispatch_queue::erase_task(const TaskPtr &task, bool force) {
	if (!force && task->requisite_for.size()) {
		using namespace StE::Text::Attributes;

		std::cout << Text::AttributedString("Attempting to remove task from GPU dispatch queue, however task ") + i(task->task_name()) + " is a requisite for " + i((*task->requisite_for.begin())->task_name()) + "." << std::endl;
		ste_log_fatal() << "task " << task->task_name() << " is a requisite for " << (*task->requisite_for.begin())->task_name() << std::endl;
		assert(false && "Task is a requisite!");
		return;
	}
	else if (force)
		task->requisite_for.clear();

	task->inserted_into_queue = false;
	task->override_fbo = nullptr;
	task->set_parent_queue(nullptr);

	Base::erase_vertex(task);

	for (auto &t : task->dependencies) {
		t->requisite_for.erase(task.get());
		if (!t->inserted_into_queue && !t->requisite_for.size())
			erase_task(t, false);
	}
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

	if (Base::get_vertices().find(dep) == Base::get_vertices().end())
		insert_task(dep, nullptr, false);
	else
		Base::erase_edge(task.get(), dep.get());

	dep->requisite_for.insert(task.get());
}

void gpu_task_dispatch_queue::delete_dep(const TaskPtr &task, const TaskPtr &dep) {
	task->dependencies.erase(dep);
	dep->requisite_for.erase(task.get());

	for (auto &s : task->sub_tasks)
		delete_dep(s, dep);

	if (Base::get_vertices().find(task) != Base::get_vertices().end() &&
		Base::get_vertices().find(dep) != Base::get_vertices().end())
		Base::add_edge(gpu_state_transition::transition_function(task.get(), dep.get()));
}

bool gpu_task_dispatch_queue::update_modified_tasks() {
	bool modified = false;
	std::unique_ptr<modify_task_log> l;
	while ((l = modify_queue.pop()) != nullptr) {
		(*l)();
		modified = true;
	}
	return modified;
}

void gpu_task_dispatch_queue::dispatch() {
	if (!modify_queue.is_empty_hint()) {
		auto mg = interrupt_optimizer_and_acquire_mutex();
		update_modified_tasks();
		clear_sop_solution_unlock_and_notify(std::move(mg));
	}
	else if (!current_optimizer.is_valid_hint(std::memory_order_relaxed) &&
			 sop.get_no_improvements_counter() < max_optimizer_runs_without_improvement)
		optimizer_notifier.notify_one();

	auto solution = sop.get_solution();
	if (!solution) {
		assert(false);
		ste_log_error() << "No solution available during dispatch!" << std::endl;
		return;
	}

	for (auto &p : solution.get().route)
		p.second->dispatch();
}
