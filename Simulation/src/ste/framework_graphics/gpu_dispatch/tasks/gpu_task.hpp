// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "FramebufferObject.hpp"

#include "sop_vertex.hpp"

#include <functional>
#include <memory>

#include <unordered_set>
#include <algorithm>

#include <string>
#include <typeinfo>

namespace StE {
namespace Graphics {

class gpu_task_dispatch_queue;
class gpu_state_transition;

class gpu_task : public Algorithm::SOP::sop_vertex<std::unordered_set<const gpu_task*>> {
private:
	friend class gpu_task_dispatch_queue;
	friend class gpu_state_transition;
	
public:
	using TaskT = const gpu_task;
	using TaskPtr = TaskT*;
	using TasksCollection = std::unordered_set<TaskPtr>;
	
private:
	std::vector<std::unique_ptr<gpu_task>> sub_tasks;
	mutable TasksCollection dependencies;
	
protected:
	mutable TasksCollection requisite_for;

private:
	// For gpu_task_dispatch_queue
	mutable bool inserted_into_queue{ false };
	mutable const Core::GenericFramebufferObject *override_fbo{ nullptr };
	mutable gpu_task_dispatch_queue *parent_queue{ nullptr };
	
	void set_override_fbo(const Core::GenericFramebufferObject *fbo) const {
		override_fbo = fbo;
		for (auto &s : sub_tasks)
			s->set_override_fbo(fbo);
		set_modified();
	}
	auto get_override_fbo() const { return override_fbo; }

	void set_parent_queue(gpu_task_dispatch_queue *q) const {
		parent_queue = q;
		for (auto &s : sub_tasks)
			s->set_parent_queue(q);
	}
	auto get_parent_queue() const { return parent_queue; }

protected:	
	void set_modified() const;
	
	void operator()() const {
		set_context_state();
		dispatch();
	}

public:
	const TasksCollection &get_dependencies() const override final { return dependencies; }
	const TasksCollection &get_requisite_for() const override final { return requisite_for; }
	
	void add_dependency(const TaskPtr &task) const {
		if (task != this) {
			dependencies.insert(task);
			for (auto &s : sub_tasks)
				s->add_dependency(task);
			set_modified();
		}
	}
	void remove_dependency(const TaskPtr &task) const {
		dependencies.erase(task);
		for (auto &s : sub_tasks)
			s->remove_dependency(task);
		set_modified();
		
		task->requisite_for.erase(this);
	}

public:
	gpu_task() = default;
	gpu_task(std::vector<std::unique_ptr<gpu_task>> &&st) : sub_tasks(std::move(st)) {
		for (auto &s : sub_tasks)
			dependencies.insert(s.get());
	}
	gpu_task(std::unique_ptr<gpu_task> &&s) : sub_tasks(1) {
		dependencies.insert(s.get());
		sub_tasks[0] = std::move(s);
	}
	virtual ~gpu_task() noexcept {}
	
	gpu_task(gpu_task &&) = default;
	gpu_task &operator=(gpu_task &&) = default;
	
protected:
	// Should set up gl resources, states, etc..
	virtual void set_context_state() const {
		if (override_fbo)
			override_fbo->bind();
	};
	// Should update buffers, locks, etc. and call a single render/compute method.
	virtual void dispatch() const = 0;
	
public:
	std::string get_name() const override final { return this->task_name(); }
	virtual std::string task_name() const { return typeid(*this).name(); }
};

}
}
