// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "FramebufferObject.hpp"

#include "sop_vertex.hpp"

#include <functional>
#include <memory>
#include <algorithm>

#include <string>
#include <typeinfo>

#include <boost/container/flat_set.hpp>

namespace StE {
namespace Graphics {

class gpu_task_dispatch_queue;
class gpu_state_transition;

class gpu_task : public Algorithm::SOP::sop_vertex<boost::container::flat_set<std::shared_ptr<const gpu_task>>,
												   boost::container::flat_set<const gpu_task*>>,
				 private std::enable_shared_from_this<gpu_task> {
private:
	friend class gpu_task_dispatch_queue;

public:
	using TaskT = const gpu_task;
	using TaskPtr = std::shared_ptr<TaskT>;
	using TaskCollection = boost::container::flat_set<std::shared_ptr<TaskT>>;

private:
	std::vector<TaskPtr> sub_tasks;
	mutable TaskCollection dependencies;

protected:
	mutable boost::container::flat_set<TaskT*> requisite_for;

private:
	// For gpu_task_dispatch_queue
	mutable bool inserted_into_queue{ false };
	mutable gpu_task_dispatch_queue *parent_queue{ nullptr };

	void set_parent_queue(gpu_task_dispatch_queue *q) const {
		parent_queue = q;
		for (auto &s : sub_tasks)
			s->set_parent_queue(q);
	}

protected:
	void operator()() const {
		set_context_state();
		dispatch();
	}

public:
	const TaskCollection &get_dependencies() const override final { return dependencies; }
	const boost::container::flat_set<TaskT*> &get_requisite_for() const override final { return requisite_for; }

	auto get_parent_queue() const { return parent_queue; }

	void add_dependency(const TaskPtr &task) const;
	void remove_dependency(const TaskPtr &task) const;

public:
	gpu_task() = default;
	gpu_task(std::vector<TaskPtr> &&st) : sub_tasks(st) {
		for (auto &s : sub_tasks)
			dependencies.insert(s);
	}
	gpu_task(const TaskPtr &s) : sub_tasks(1) {
		dependencies.insert(s);
		sub_tasks[0] = s;
	}
	virtual ~gpu_task() noexcept {}

	gpu_task(gpu_task &&) = default;
	gpu_task &operator=(gpu_task &&) = default;

protected:
	// Should set up gl resources, states, etc..
	virtual void set_context_state() const = 0;
	// Should update buffers, locks, etc. and call a single render/compute method.
	virtual void dispatch() const = 0;

public:
	std::string get_name() const override final { return this->task_name(); }
	virtual std::string task_name() const { return typeid(*this).name(); }
};

}
}
