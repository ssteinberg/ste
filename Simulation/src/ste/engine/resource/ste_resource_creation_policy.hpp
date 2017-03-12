//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <future>
#include <memory>
#include <functional>

namespace StE {

struct ste_resource_dont_defer {};

struct ste_resource_async_policy_std_async {
	template <typename L>
	static auto async(const ste_context &ctx, L&& lambda) {
		return std::async(std::forward<L>(lambda));
	}
};

struct ste_resource_async_policy_task_scheduler {
	template <typename L>
	static auto async(const ste_context &ctx, L&& lambda) {
		return ctx.engine().task_scheduler().schedule_now(std::forward<L>(lambda)).get_future();
	}
};

/**
*	@brief	Async resource creation policy. 
*	The resource will be constructed on a background worker thread.
*
*	@param	async_policy	Policy that dictates the creation of the background thread
*/
template <class async_policy>
class ste_resource_deferred_creation_policy_async {
	using lambda_t = std::function<void(void)>;
	struct creator_t {
		std::future<void> future;
	};

	creator_t creator;

public:
	void consume() {
		creator.future.wait();
	}

	ste_resource_deferred_creation_policy_async() = default;
	template <typename... Ts>
	ste_resource_deferred_creation_policy_async(Ts&&... ts) : creator{
		async_policy::async(std::forward<Ts>(ts)...)
	} {}
};

/**
*	@brief	Lazy resource creation policy. 
*			Resource will be instantiated only once a getter is called.
*/
class ste_resource_deferred_creation_policy_lazy {
	using lambda_t = std::function<void(void)>;
	struct creator_t {
		std::mutex m;
		std::unique_ptr<lambda_t> lambda{ nullptr };

		creator_t() = default;
		creator_t(std::unique_ptr<lambda_t> &&lambda) : lambda(std::move(lambda)) {}
	};

	creator_t creator;

public:
	void consume() {
		std::unique_lock<std::mutex> l(creator.m);
		if (creator.lambda != nullptr) {
			(*creator.lambda)();
			creator.lambda = nullptr;
		}
	}

	ste_resource_deferred_creation_policy_lazy() = default;
	template <typename L>
	ste_resource_deferred_creation_policy_lazy(const ste_context &, L&& lambda) : creator{
		std::make_unique<lambda_t>(std::forward<L>(lambda))
	} {}
};

}
