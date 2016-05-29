// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "resource_loading_task.hpp"

#include <memory>
#include <functional>

namespace StE {
namespace Resource {

template <typename R>
class resource_instance {
public:
	using loader_future_type = task_future<std::unique_ptr<R>>;

private:
	mutable std::unique_ptr<R> resource{ nullptr };
	mutable std::unique_ptr<loader_future_type> loader_future{ nullptr };

public:
	resource_instance() = default;
	template <typename ... Ts>
	resource_instance(const StEngineControl &ctx, Ts&&... args) {
		load(ctx, std::forward<Ts>(args)...);
	}

	resource_instance(resource_instance &&) = default;
	resource_instance(const resource_instance &) = delete;
	resource_instance &operator=(resource_instance &&) = default;
	resource_instance &operator=(const resource_instance &) = delete;

	template <typename ... Ts>
	void load(const StEngineControl &ctx, Ts&&... args) {
		auto f = resource_loading_task<R>().loader(ctx, std::forward<Ts>(args)...);
		loader_future = std::make_unique<loader_future_type>(std::move(f));
	}

	void wait() const {
		if (resource == nullptr) {
			assert(loader_future != nullptr && "load() wasn't called!");

			resource = loader_future->get();
			assert(resource != nullptr && "loader returned nullptr!");

			loader_future = nullptr;
		}
	}

	auto &get() {
		wait();
		return *resource.get();
	}
	const auto &get() const {
		wait();
		return *resource.get();
	}

	auto &future() const {
		assert(loader_future != nullptr && "future() called after get() or before load()");
		return *loader_future;
	}
};

}
}
