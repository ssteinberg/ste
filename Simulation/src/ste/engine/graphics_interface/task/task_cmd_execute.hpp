//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <command.hpp>

namespace ste {
namespace gl {

namespace _internal {

// Binds a pipeline, or does nothing if policy has no pipeline
template <typename Task, typename pipeline_policy, typename = std::void_t<>>
struct task_cmd_execute_bind_pipeline {
	template <typename Token>
	void operator()(const Task *task,
					Token,
					command_recorder &recorder) const {}
};
template <typename Task, typename pipeline_policy>
struct task_cmd_execute_bind_pipeline<Task, pipeline_policy, std::void_t<typename pipeline_policy::pipeline_object_type>> {
	template <typename Token>
	void operator()(const Task *task,
					Token,
					command_recorder &recorder) const {
		auto *pipeline_object = task->get_pipeline_storage(Token()).obj;
		assert(pipeline_object != nullptr && "Unattached pipeline object");
		recorder << pipeline_object->cmd_bind();
	}
};

// Unbinds a pipeline, or does nothing if policy has no pipeline
template <typename Task, typename pipeline_policy, typename = std::void_t<>>
struct task_cmd_execute_unbind_pipeline {
	template <typename Token>
	void operator()(const Task *task,
					Token,
					command_recorder &recorder) const {}
};
template <typename Task, typename pipeline_policy>
struct task_cmd_execute_unbind_pipeline<Task, pipeline_policy, std::void_t<typename pipeline_policy::pipeline_object_type>> {
	template <typename Token>
	void operator()(const Task *task,
					Token,
					command_recorder &recorder) const {
		auto *pipeline_object = task->get_pipeline_storage(Token()).obj;
		assert(pipeline_object != nullptr && "Unattached pipeline object");
		recorder << pipeline_object->cmd_unbind();
	}
};

template <typename Task, typename Command, typename pipeline_policy>
class task_cmd_execute : public command {
	const Task *task;
	Command cmd;

public:
	template <typename... CmdArgs>
	task_cmd_execute(const Task *task,
					 CmdArgs&&... cmd_args)
		: task(task),
		// Constructs the task command, directly, or using a custom creator if the policy provides one
		cmd(Task::policy::template create_cmd<Command>(task,
													   std::forward<CmdArgs>(cmd_args)...))
	{}
	virtual ~task_cmd_execute() noexcept {}

	task_cmd_execute(task_cmd_execute&&) = default;
	task_cmd_execute &operator=(task_cmd_execute&&) = default;

private:
	void operator()(const command_buffer &,
					command_recorder &recorder) const override final {
		// Bind the pipeline, if any
		task_cmd_execute_bind_pipeline<Task, pipeline_policy>()(task, Task::accessor_token(), recorder);

		// Prepare the task (bind buffers, etc.)
		Task::policy::prepare(task, recorder);
		// Record the command itself
		recorder << cmd;

		// Unbind the pipeline
		task_cmd_execute_unbind_pipeline<Task, pipeline_policy>()(task, Task::accessor_token(), recorder);
	}
};

}

}
}
