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
	using resource_instance_future = task_future<void>;

private:
	template <class, class> class checker;
	template <typename C>
	static std::true_type loader_test(checker<C, decltype(&C:: template loader<const StEngineControl &, R*>)> *);
	template <typename C>
	static std::false_type loader_test(...);

public:
	resource_instance_future loader(const StEngineControl &ctx, R* res) {
		static_assert(std::is_same<decltype(loader_test<R>(ctx, res)), std::true_type>::value,
					  "To comply with resource_loading_task idiom R must implement loader(const StEngineControl &, R*) method, which schedules the loading task. Or partially specialize resource_loading_task<R>.");

		return R::loader(ctx, res);
	}
};

}
}
