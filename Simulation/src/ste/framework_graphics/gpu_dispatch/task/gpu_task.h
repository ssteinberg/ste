// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "FramebufferObject.h"

#include <unordered_set>
#include <memory>

#include <string>
#include <typeinfo>

namespace StE {
namespace Graphics {

class gpu_task_dispatch_queue;

class gpu_task : private std::enable_shared_from_this<gpu_task> {
private:
	friend class gpu_task_dispatch_queue;
	
public:
	using TaskT = const gpu_task;
	using TaskPtr = std::shared_ptr<TaskT>;
	using TasksCollection = std::unordered_set<TaskPtr>;

private:
	mutable TasksCollection after;
	mutable TasksCollection dependencies;
	
	mutable TasksCollection requisite_for, task_dependencies;
	mutable bool inserted_into_queue{ false };
	mutable const LLR::GenericFramebufferObject *override_fbo{ nullptr };
	mutable gpu_task_dispatch_queue *parent_queue { nullptr };
	
protected:
	TasksCollection sub_tasks;
	
private:
	void set_override_fbo(const LLR::GenericFramebufferObject *fbo) const {
		override_fbo = fbo;
		set_modified();
	}
	auto get_override_fbo() const { return override_fbo; }

protected:
	const auto &get_dependencies() const { return dependencies; }
	
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
	virtual std::string task_name() const { return typeid(*this).name(); }
};

}
}
