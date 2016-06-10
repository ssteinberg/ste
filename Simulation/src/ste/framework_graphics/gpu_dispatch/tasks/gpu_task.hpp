// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "sop_vertex.hpp"
#include "sop_edge.hpp"

#include "profileable.hpp"

#include "gpu_dispatchable.hpp"
#include "FramebufferObject.hpp"

#include <functional>
#include <memory>
#include <algorithm>
#include <string>

#include "boost_flatset.hpp"

namespace StE {
namespace Graphics {

class gpu_task_dispatch_queue;
class gpu_state_transition;
class gpu_task_factory;

class gpu_task : public Algorithm::SOP::sop_vertex<boost::container::flat_set<std::shared_ptr<const gpu_task>>,
												   boost::container::flat_set<const gpu_task*>>,
				 public std::enable_shared_from_this<gpu_task>,
				 public profileable {
private:
	friend class gpu_task_dispatch_queue;
	friend class gpu_state_transition;
	friend class gpu_task_factory;

protected:
	struct AccessToken {};

public:
	using TaskT = const gpu_task;
	using TaskPtr = std::shared_ptr<TaskT>;
	using TaskCollection = boost::container::flat_set<std::shared_ptr<TaskT>>;

private:
	std::string name;

	const gpu_dispatchable *dispatchable;
	mutable const Core::GenericFramebufferObject *fbo{ nullptr };

	std::vector<TaskPtr> sub_tasks;
	mutable TaskCollection dependencies;
	mutable boost::container::flat_set<TaskT*> requisite_for;

private:
	// For gpu_task_dispatch_queue
	mutable bool inserted_into_queue{ false };
	mutable gpu_task_dispatch_queue *parent_queue{ nullptr };

	void set_parent_queue(gpu_task_dispatch_queue *q) const {
		parent_queue = q;
	}

protected:
	void set_fbo(const Core::GenericFramebufferObject *fbo) const;

public:
	gpu_task(AccessToken,
			 const std::string &name,
			 const gpu_dispatchable *dispatchable) : name(name), dispatchable(dispatchable) {}
	gpu_task(AccessToken,
			 const std::string &name,
			 const gpu_dispatchable *dispatchable,
			 const Core::GenericFramebufferObject *fbo) : gpu_task(AccessToken(), name, dispatchable) {
		this->fbo = fbo;
	}
	gpu_task(AccessToken,
			 const std::string &name,
			 const gpu_dispatchable *dispatchable,
			 const Core::GenericFramebufferObject *fbo,
			 std::vector<TaskPtr> &&st) : gpu_task(AccessToken(), name, dispatchable, fbo) {
		sub_tasks = std::move(st);
		for (auto &s : sub_tasks)
			dependencies.insert(s);
	}

	gpu_task(gpu_task &&) = default;
	gpu_task &operator=(gpu_task &&) = default;
	virtual ~gpu_task() noexcept {}

	std::string get_name() const override final { return name; }

	const TaskCollection &get_dependencies() const override final { return dependencies; }
	const boost::container::flat_set<TaskT*> &get_requisite_for() const override final { return requisite_for; }

	auto get_parent_queue() const { return parent_queue; }
	auto get_fbo() const { return fbo; }

	void add_dependency(const TaskPtr &task) const;
	void remove_dependency(const TaskPtr &task) const;

private:
	void set_context_state() const {
		dispatchable->set_context_state();
		if (fbo)
			fbo->bind();
	}

	inline void dispatch() const {
		dispatchable->dispatch();
	}
};

}
}

#include "gpu_task_factory.hpp"
