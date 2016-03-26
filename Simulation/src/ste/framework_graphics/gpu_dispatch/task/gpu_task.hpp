// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "FramebufferObject.hpp"

#include "graph_vertex.hpp"

#include <unordered_set>
#include <memory>

#include <string>
#include <typeinfo>

namespace StE {
namespace Graphics {

class gpu_task_dispatch_queue;
class gpu_state_transition;

class gpu_task : public Graph::vertex, private std::enable_shared_from_this<gpu_task> {
private:
	friend class gpu_task_dispatch_queue;
	friend class gpu_state_transition;
	
public:
	using TaskT = const gpu_task;
	using TaskPtr = std::shared_ptr<TaskT>;
	using TasksCollection = std::unordered_set<TaskPtr>;

protected:
	TasksCollection sub_tasks;
	mutable TasksCollection after;
	mutable TasksCollection task_dependencies;
	
private:
	// For gpu_task_dispatch_queue
	mutable TasksCollection requisite_for, dependencies, parent_deps;
	mutable bool inserted_into_queue{ false };
	mutable const Core::GenericFramebufferObject *override_fbo{ nullptr };
	mutable gpu_task_dispatch_queue *parent_queue { nullptr };
	
private:
	void set_override_fbo(const Core::GenericFramebufferObject *fbo) const {
		override_fbo = fbo;
		set_modified();
	}
	auto get_override_fbo() const { return override_fbo; }

protected:
	const auto &get_dependencies() const { return task_dependencies; }
	
	void set_modified() const;
	
	void operator()() const {
		set_context_state();
		dispatch();
	}

public:
	void add_dependency(const TaskPtr &task) const {
		task_dependencies.insert(task);
		set_modified();
	}
	void remove_dependency(const TaskPtr &task) const {
		task_dependencies.erase(task);
		set_modified();
	}
	
	void add_after(const TaskPtr &task) const {
		after.insert(task);
		set_modified();
	}
	void remove_after(const TaskPtr &task) const {
		after.erase(task);
		set_modified();
	}

public:
	virtual ~gpu_task() noexcept {}
	
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
