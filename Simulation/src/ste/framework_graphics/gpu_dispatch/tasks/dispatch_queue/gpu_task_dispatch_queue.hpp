// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_task.hpp"
#include "gpu_task_root.hpp"
#include "gpu_state_transition.hpp"
#include "sop_graph.hpp"

#include "FramebufferObject.hpp"

#include "sequential_ordering_problem.hpp"

#include "concurrent_queue.hpp"
#include "shared_double_reference_guard.hpp"
#include "interruptible_thread.hpp"

#include <mutex>
#include <atomic>
#include <condition_variable>

#include <functional>
#include <memory>

namespace StE {
namespace Graphics {

class gpu_task_dispatch_queue : private Algorithm::SOP::sop_graph<gpu_task, gpu_state_transition> {
	using Base = Algorithm::SOP::sop_graph<gpu_task, gpu_state_transition>;

	static constexpr unsigned max_optimizer_runs_without_improvement = 50;
	static constexpr unsigned optimizer_iterations = 5;

private:
	using TaskT = typename gpu_task::TaskT;
	using TaskPtr = typename gpu_task::TaskPtr;
	using TaskCollection = typename gpu_task::TaskCollection;

	using sop_type = Algorithm::SOP::sequential_ordering_problem<Base>;
	using sop_optimizer_type = sop_type::optimizer_type;
	using sop_optimizer_double_ref_guard = shared_double_reference_guard<sop_optimizer_type, false>;

	using modify_task_log = std::function<void()>;

private:
	mutable concurrent_queue<modify_task_log> modify_queue;
	std::shared_ptr<detail::gpu_task_root> root;

	sop_type sop;
	sop_optimizer_double_ref_guard current_optimizer;
	interruptible_thread optimizer_thread;
	mutable std::mutex optimizer_notification_mutex, optimizer_mutex;
	std::condition_variable optimizer_notifier;
	std::atomic<bool> stopping_optimizer{ false };

private:
	void insert_task(const TaskPtr &task, bool);
	void erase_task(const TaskPtr&, bool);
	void erase_all();
	void add_dep(const TaskPtr &task, const TaskPtr &dep);
	void delete_dep(const TaskPtr &task, const TaskPtr &dep);

	void build_task_transitions(const TaskPtr&);

	std::unique_lock<std::mutex> interrupt_optimizer_and_acquire_mutex();
	void clear_sop_solution_unlock_and_notify(std::unique_lock<std::mutex> &&);

protected:
	bool update_modified_tasks();

public:
	gpu_task_dispatch_queue();
	~gpu_task_dispatch_queue() noexcept;

	void add_task(const TaskPtr &task) {
		modify_queue.push(modify_task_log([this, task = std::move(task)]() {
			this->insert_task(std::move(task), true);
		}));
	}

	void remove_task(const TaskPtr &task) {
		modify_queue.push(modify_task_log([=]() {
			this->erase_task(task, false);
		}));
	}

	void add_task_dependency(const TaskPtr &task, const TaskPtr &dep) {
		assert(dep != task);
		modify_queue.push(modify_task_log([=]() {
			this->add_dep(task, dep);
		}));
	}

	void remove_task_dependency(const TaskPtr &task, const TaskPtr &dep) {
		assert(dep != task);
		modify_queue.push(modify_task_log([=]() {
			this->delete_dep(task, dep);
		}));
	}

	void remove_all() {
		modify_queue.push(modify_task_log([=]() {
			this->erase_all();
		}));
	}

	void dispatch();

	using Base::write_dot;
	using Base::write_png;
};

}
}
