//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <task_policy.hpp>
#include <task_command.hpp>

#include <job.hpp>

#include <forward_capture.hpp>
#include <tuple_call.hpp>
#include <type_traits>
#include <lib/string.hpp>

namespace ste {
namespace gl {

namespace _internal {

template <typename pipeline_policy, typename = std::void_t<>>
struct task_impl_pipeline_storage {};
template <typename pipeline_policy>
struct task_impl_pipeline_storage<pipeline_policy, std::void_t<typename pipeline_policy::pipeline_object_type>> {
	using pipeline_t = typename pipeline_policy::pipeline_object_type;
	pipeline_t *obj{ nullptr };
};

template <class Command, typename task_policy, typename pipeline_policy>
class task_impl : public task_policy::interface {
public:
	using policy = task_policy;

private:
	using pipeline_storage_t = task_impl_pipeline_storage<pipeline_policy>;
	using command_t = task_command<task_impl<Command, task_policy, pipeline_policy>, Command, pipeline_policy>;

	friend command_t;
	template <class pipeline_object_type>
	struct task_pipeline_policy_extract_pipeline_resources;

	struct accessor_token {};

private:
	pipeline_storage_t pipeline_storage;
	lib::string task_name;

public:
	auto& get_pipeline_storage(accessor_token) const { return pipeline_storage; }

public:
	task_impl() = default;
	task_impl(lib::string &&name) : task_name(std::move(name)) {}
	~task_impl() noexcept {}

	task_impl(task_impl&&) = default;
	task_impl &operator=(task_impl&&) = default;

	const auto &name() const { return task_name; }

	/**
	*	@brief	The task's requested queue type. The task should be executed on a compatible queue.
	*/
	auto requested_queue_type() const { return pipeline_policy::queue_type; }

	/**
	*	@brief	Binds a pipeline object to the task.
	*			Only available for tasks that require a pipeline
	*/
	template <typename pp = pipeline_policy>
	void attach_pipeline(typename pp::pipeline_object_type &pipeline_object) {
		pipeline_storage.obj = &pipeline_object;
	}

	/**
	*	@brief	Gets the currently attached pipeline object.
	*			Only available for tasks that require a pipeline
	*/
	template <typename pp = pipeline_policy>
	const typename pp::pipeline_object_type &get_attached_pipeline() const {
		return *pipeline_storage.obj;
	}

	/**
	*	@brief	Creates a command that executes the task.
	*/
	template <typename... CmdArgs>
	command_t operator()(CmdArgs&&... cmd_args) const {
		return command_t(this,
						 std::forward<CmdArgs>(cmd_args)...);
	}

	/**
	*	@brief	Executes, consuming the task in the process.
	*/
	template <typename... CmdArgs>
	auto execute(const ste_context &ctx,
				 CmdArgs&&... cmd_args) && {
		auto future = ctx.engine().task_scheduler().schedule_now([&ctx,
																 task = std::move(*this),
																 pack = forward_capture_pack(std::forward<CmdArgs>(cmd_args)...)]() mutable {
			// Create command
			auto cmd = tuple_call(&task,
								  &decltype(task)::template operator()<CmdArgs...>,
								  std::move(pack));

			// Select queue
			auto queue_type = task.requested_queue_type();
			auto queue_selector = ste_queue_selector<ste_queue_selector_policy_flexible>(queue_type);
			auto &q = ctx.device().select_queue(queue_selector);

			// Enqueue execute command
			auto enqueue_future = q.enqueue([cmd = std::move(cmd)]() mutable {
				auto batch = ste_device_queue::thread_allocate_batch();
				auto& command_buffer = batch->acquire_command_buffer();

				// Record and submit a one-time batch
				{
					auto recorder = command_buffer.record();
					recorder << std::move(cmd);
				}

				ste_device_queue::submit_batch(std::move(batch));
			});

			enqueue_future.get();
		});

		return make_job(std::move(future));
	}
	/**
	*	@brief	Executes the task.
	*/
	template <typename... CmdArgs>
	auto execute(const ste_context &ctx,
				 CmdArgs&&... cmd_args) const& {
		// For safety, we must take a copy of the task
		auto new_task = *this;
		return std::move(new_task).execute(ctx, std::forward<CmdArgs>(cmd_args)...);
	}
};

}

template <class Command>
using task = _internal::task_impl<
	Command,
	task_policy<Command>,
	typename task_policy<Command>::pipeline_policy
>;

template <class Command, typename... CmdArgs>
auto execute(const ste_context &ctx,
			 const task<Command> &task, 
			 CmdArgs&&... cmd_args) {
	return task.execute(ctx,
						std::forward<CmdArgs>(cmd_args)...);
}
template <class Command, typename... CmdArgs>
auto execute(const ste_context &ctx,
			 task<Command> &&task, 
			 CmdArgs&&... cmd_args) {
	return std::move(task).execute(ctx,
								   std::forward<CmdArgs>(cmd_args)...);
}

}
}
