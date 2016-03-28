// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_task.hpp"
#include "gpu_task_root.hpp"
#include "gpu_state_transition.hpp"
#include "sop_graph.hpp"

#include "FramebufferObject.hpp"

#include <memory>
#include <unordered_set>
#include <vector>

namespace StE {
namespace Graphics {

class gpu_task_dispatch_queue : private Algorithm::SOP::sop_graph<gpu_task, gpu_state_transition> {
	using Base = Algorithm::SOP::sop_graph<gpu_task, gpu_state_transition>;
	
	friend class gpu_task;
	
private:
	using TaskT = typename gpu_task::TaskT;
	using TaskPtr = typename gpu_task::TaskPtr;
	using TasksCollection = typename gpu_task::TasksCollection;

private:
	std::vector<TaskPtr> modified_tasks;
	std::unique_ptr<detail::gpu_task_root> root;

private:
	void add_task(const TaskPtr&, const Core::GenericFramebufferObject*, bool);
	void build_task_transitions(const TaskPtr&);
	
protected:
	void update_modified_tasks();
	void signal_task_modified(const TaskPtr &task) { modified_tasks.push_back(task); }

public:
	gpu_task_dispatch_queue();
	~gpu_task_dispatch_queue() noexcept {}

	void add_task(const TaskPtr &task, const Core::GenericFramebufferObject *override_fbo = nullptr);
	void remove_task(const TaskPtr &task, bool force = false);
	void update_task_fbo(const TaskPtr &task, const Core::GenericFramebufferObject *override_fbo) const;
	
	void remove_all();

	void dispatch();
	
	using Base::write_dot;
	using Base::write_png;
};

}
}
