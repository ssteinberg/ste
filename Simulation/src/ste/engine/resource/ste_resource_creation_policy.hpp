//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <future>
#include <lib/unique_ptr.hpp>
#include <functional>

#include <optional.hpp>

namespace ste {

struct ste_resource_dont_defer {};
struct ste_resource_create_with_lambda {};

struct ste_resource_async_policy_std_async {
	template <typename T>
	using future = std::future<T>;

	template <typename L>
	static auto async(const ste_context &ctx, L&& lambda) {
		return std::async(std::forward<L>(lambda));
	}
};

struct ste_resource_async_policy_task_scheduler {
	template <typename T>
	using future = task_future<T>;

	template <typename L>
	static auto async(const ste_context &ctx, L&& lambda) {
		return ctx.engine().task_scheduler().schedule_now(std::forward<L>(lambda));
	}
};

/**
*	@brief	Async resource creation policy. 
*	The resource will be constructed on a background worker thread.
*
*	@param	async_policy		Policy that dictates the creation of the background thread
*/
template <class async_policy>
struct ste_resource_deferred_creation_policy_async {
	template <typename T>
	class policy {
	private:
		std::mutex m;
		optional<typename async_policy::template future<T>> future;

	public:
		policy() = default;
		template <typename L>
		policy(const ste_context &ctx, L&& lambda) : future(async_policy::async(ctx, std::forward<L>(lambda))) {}

		policy(policy &&o) noexcept {
			std::unique_lock<std::mutex> l(o.m);
			future = std::move(o.future);
		}
		policy &operator=(policy &&o) noexcept {
			future = std::move(o.future);
			return *this;
		}

		void consume(T &t) {
			std::unique_lock<std::mutex> l(m);
			if (future) {
				t = future.get().get();
				future = none;
			}
		}
	};
};

/**
*	@brief	Lazy resource creation policy. 
*			Resource will be instantiated only once a getter is called.
*/
struct ste_resource_deferred_creation_policy_lazy {
	template <typename T>
	class policy {
	using lambda_t = std::function<T(void)>;
	private:
		std::mutex m;
		optional<lambda_t> lambda;

	public:
		policy() = default;
		template <typename L>
		policy(const ste_context &, L&& lambda) : lambda(std::forward<L>(lambda)) {}

		policy(policy &&o) noexcept {
			std::unique_lock<std::mutex> l(o.m);
			lambda = std::move(o.lambda);
		}
		policy &operator=(policy &&o) noexcept {
			lambda = std::move(o.lambda);
			return *this;
		}

		void consume(T &t) {
			std::unique_lock<std::mutex> l(m);
			if (lambda) {
				t = lambda.get()();
				lambda = none;
			}
		}
	};
};

}
