// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "task_future.hpp"

#include <memory>
#include <type_traits>

namespace StE {
namespace Resource {

template <typename R>
class resource_loading_task {
private:
	using resource_instance_future = task_future<std::unique_ptr<R>>;

public:
	template <typename ... Ts>
	resource_instance_future loader(const StEngineControl &ctx, Ts&&... args) {
		static_assert(std::is_same<decltype(R::loader(ctx, std::forward<Ts>(args)...)), resource_instance_future>::value,
					  "To comply with resource_loading_task idiom R must implement loader(const StEngineControl &, Ts...) method, \
					   which schedules the loading task and returns task_future<std::unique_ptr<R>>.");

		return R::loader(ctx, std::forward<Ts>(args)...);
	}
};

template <typename R>
class resource_loading_guard {
	using future_type = task_future<std::unique_ptr<R>>;

private:
	future_type *f;

public:
	resource_loading_guard(future_type &f) : f(&f) {}
	auto &operator=(future_type &f) {
		this->f = &f;
		return *this;
	}

	resource_loading_guard(resource_loading_guard &&) = default;
	resource_loading_guard(const resource_loading_guard &) = delete;
	resource_loading_guard &operator=(resource_loading_guard &&) = default;
	resource_loading_guard &operator=(const resource_loading_guard &) = delete;

	~resource_loading_guard() {
		if (f)
			f.wait();
	}
};

}
}
