// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "resource_loading_task.hpp"
#include "resource_instance_getter.hpp"

#include <memory>
#include <functional>

namespace StE {
namespace Resource {

template <typename R>
class resource_instance {
public:
	using loader_future_type = task_future<void>;

private:
	R resource;
	mutable std::unique_ptr<loader_future_type> loader_future{ nullptr };

public:
	template <typename ... Ts>
	resource_instance(const StEngineControl &ctx, Ts&&... args) : resource(ctx, std::forward<Ts>(args)...) {
		auto f = resource_loading_task<R>().loader(ctx, &resource);
		loader_future = std::make_unique<loader_future_type>(std::move(f));
	}

	resource_instance(resource_instance &&) = default;
	resource_instance(const resource_instance &) = delete;
	resource_instance &operator=(resource_instance &&) = default;
	resource_instance &operator=(const resource_instance &) = delete;

	void wait() const {
		if (loader_future != nullptr) {
			loader_future->get();
			loader_future = nullptr;
		}
	}

	auto &get() {
		wait();
		return resource_instance_getter<R>().get(&resource);
	}
	const auto &get() const {
		wait();
		return resource_instance_getter<R>().get(&resource);
	}

	auto &future() const {
		assert(loader_future != nullptr && "future() called after get() or before load()");
		return *loader_future;
	}
};

}
}
