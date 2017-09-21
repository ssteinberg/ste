//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <command.hpp>
#include <task_foreach_interface.hpp>

namespace ste {
namespace gl {

namespace _internal {

// Binds a pipeline, or does nothing if policy has no pipeline
template <typename Task, typename pipeline_policy, typename = std::void_t<>>
struct tast_command_bind_pipeline {
	template <typename Token>
	void operator()(const Task *task,
					Token,
					command_recorder &recorder) const {}
};
template <typename Task, typename pipeline_policy>
struct tast_command_bind_pipeline<Task, pipeline_policy, std::void_t<typename pipeline_policy::pipeline_object_type>> {
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
struct tast_command_unbind_pipeline {
	template <typename Token>
	void operator()(const Task *task,
					Token,
					command_recorder &recorder) const {}
};
template <typename Task, typename pipeline_policy>
struct tast_command_unbind_pipeline<Task, pipeline_policy, std::void_t<typename pipeline_policy::pipeline_object_type>> {
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
class task_command : public command {
	const Task *task;
	Command cmd;

public:
	template <typename... CmdArgs>
	task_command(const Task *task,
				 CmdArgs&&... cmd_args)
		: task(task),
		// Constructs the task command, directly, or using a custom creator if the policy provides one
		cmd(Task::policy::template create_cmd<Command>(task,
													   std::forward<CmdArgs>(cmd_args)...))
	{}
	virtual ~task_command() noexcept {}

	task_command(task_command&&) = default;
	task_command &operator=(task_command&&) = default;

private:
	void operator()(const command_buffer &,
					command_recorder &recorder) && override final {
		// Bind the pipeline, if any
		tast_command_bind_pipeline<Task, pipeline_policy>()(task, Task::accessor_token(), recorder);

		// Bind the task resources (buffers, etc.)
		task_foreach_interface<Task>()(task, [&recorder](auto* interface) {
			using Interface = std::remove_cv_t<std::remove_reference_t<decltype(*interface)>>;
			task_interface_binder<Interface>()(interface, recorder);
		});

		// Record the command itself
		recorder << std::move(cmd);

		// Unbind the pipeline
		tast_command_unbind_pipeline<Task, pipeline_policy>()(task, Task::accessor_token(), recorder);
	}
};

}

}
}
