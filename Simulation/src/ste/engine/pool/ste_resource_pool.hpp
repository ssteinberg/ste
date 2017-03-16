//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_resource_pool_resource.hpp>
#include <ste_resource_pool_reclamation_policy.hpp>

#include <memory>
#include <concurrent_queue.hpp>
#include <functional>
#include <forward_capture.hpp>
#include <tuple_call.hpp>
#include <type_traits>

namespace StE {
namespace GL {

template <
	typename T, 
	template<typename> class resource_reclamation_policy = ste_resource_pool_reclamation_policy
>
class ste_resource_pool {
private:
	using value_type = T;
	using pool_t = concurrent_queue<value_type>;

public:
	using resource_t = ste_resource_pool_resource<ste_resource_pool<value_type, resource_reclamation_policy>, resource_reclamation_policy>;

	friend resource_t;

private:
	struct creator {
		template <typename... Ts>
		decltype(auto) operator()(Ts&&... params) { return std::make_unique<value_type>(std::forward<Ts>(params)...); }
	};

private:
	std::function<std::unique_ptr<value_type>(void)> res_creator;
	pool_t pool;

private:
	void release_to_pool(std::unique_ptr<value_type> &&res) {
		pool.push(std::move(res));
	}

public:
	template <typename... ResArgs>
	ste_resource_pool(ResArgs&&... res_ctor_args)
		: res_creator([pack = forward_capture_pack(std::forward<ResArgs>(res_ctor_args)...)]() {
		creator c;
		return tuple_call(&c,
						  &creator::template operator() < ResArgs... > ,
						  pack);
	})
	{}
	virtual ~ste_resource_pool() {}

	auto claim() {
		auto p = pool.pop();
		if (p != nullptr) {
			// Return from pool
			resource_reclamation_policy<value_type>::reset(*p);
			return resource_t(this, std::move(p));
		}

		// Create new
		return resource_t(this, res_creator());
	}
};

}
}
