// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "resource_loading_task.hpp"
#include "resource_instance_getter.hpp"

#include <memory>
#include <mutex>
#include <functional>

#include <stack>
#include <vector>

namespace StE {
namespace Resource {

class resource_instance_base {
private:
	static thread_local std::stack<const resource_instance_base*> resources_being_ctored_stack;

protected:
	resource_instance_base() {
		if (!resources_being_ctored_stack.empty()) {
			auto parent = resources_being_ctored_stack.top();
			parent->add_child_resource(this);
		}
		resources_being_ctored_stack.push(this);
	}

	void pop_ctor_stack() { resources_being_ctored_stack.pop(); }

	virtual void add_child_resource(const resource_instance_base *child) const = 0;

public:
	virtual void wait() const = 0;
};

/**
 *	@brief	Resource instance wrap
 *
 *	Wraps a resource R and provides async loading facilities
 *	resource_loading_task<R> defines custom loading process.
 *	resource_instance_getter<R> defines custom getters.
 *
 *	@param R	resource type
 */
template <typename R>
class resource_instance : protected resource_instance_base {
public:
	using loader_future_type = task_future<void>;

private:
	struct loading_struct {
		std::unique_ptr<loader_future_type> loader_future;
		std::vector<const resource_instance_base*> child_resources;
	};

private:
	mutable std::mutex m;
	mutable std::unique_ptr<loading_struct> loading_data;
	R resource;

private:
	void add_child_resource(const resource_instance_base *child) const override final {
		assert(loading_data != nullptr);
		loading_data->child_resources.push_back(child);
	}

public:
	/**
	*	@brief	Resource ctor
	*
	*	Calls R ctor and schedules loading task
	*
	*	@param ctx	Engine context reference
	*	@param args	Will be perfectly forwarded to resource ctor
	*/
	template <typename ... Ts>
	resource_instance(const StEngineControl &ctx, Ts&&... args) : resource_instance_base(),
																  loading_data(std::make_unique<loading_struct>()),
																  resource(ctx, std::forward<Ts>(args)...) {
		resource_instance_base::pop_ctor_stack();

		optional<loader_future_type> future = resource_loading_task<R>().loader(ctx, &resource);
		if (future)
			loading_data->loader_future = std::make_unique<loader_future_type>(std::move(future.get()));
	}

	resource_instance(resource_instance &&) = default;
	resource_instance(const resource_instance &) = delete;
	resource_instance &operator=(resource_instance &&) = default;
	resource_instance &operator=(const resource_instance &) = delete;

	/**
	*	@brief	Wait for loading to complete
	*
	*	Blocks till loading of resource and all child-resources is completed. Unhandled exceptions raised
	*	while loading are rethrown here.
	*/
	void wait() const override final {
		if (loading_data != nullptr) {
			std::unique_lock<std::mutex> ul(m);
			if (loading_data == nullptr)
				return;

			for (auto *child : loading_data->child_resources)
				child->wait();
			if (loading_data->loader_future != nullptr)
				loading_data->loader_future->get();

			loading_data = nullptr;
		}
	}

	/**
	*	@brief	Get resource reference
	*
	*	Calls wait and returns a reference to underlying resource
	*/
	auto &get() {
		wait();
		return resource_instance_getter<R>().get(&resource);
	}
	/**
	*	@brief	Get resource reference
	*
	*	Calls wait and returns a reference to underlying resource
	*/
	const auto &get() const {
		wait();
		return resource_instance_getter<R>().get(&resource);
	}
};

}
}
