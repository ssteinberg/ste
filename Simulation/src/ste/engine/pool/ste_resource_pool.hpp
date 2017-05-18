//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_resource_pool_resource.hpp>
#include <ste_resource_pool_reclamation_policy.hpp>

#include <lib/concurrent_queue.hpp>
#include <tuple_call.hpp>
#include <type_traits>

namespace ste {
namespace gl {

template <
	typename T, 
	template<class> class resource_reclamation_policy = ste_resource_pool_reclamation_policy
>
class ste_resource_pool {
	static_assert(ste_resource_pool_is_poolable<T>::value,
				  "T is not poolable. To conform to poolable trait, T must inherit from one of the ste_resource_pool_*_trait traits.");

private:
	using value_type = T;
	using pool_t = lib::concurrent_queue<value_type>;
	using tuple_t = typename ste_resource_pool_ctor_args_capture<T>::type;

public:
	using resource_ptr_t = lib::unique_ptr<T>;
	using resource_t = ste_resource_pool_resource<ste_resource_pool<value_type>, resource_ptr_t, resource_reclamation_policy>;
	using pool_ptr_t = pool_t*;

	friend resource_t;

private:
	const tuple_t res_params;
	pool_t pool;

public:
	/**
	*	@brief	Allocates a new resource pool.
	*	
	*	@param res_ctor_args	Resource ctor args. As those arguments will be used multiple times in instantiation of resources,
	*							the arguments can not be moved and must be copy-contructible.
	*/
	template <typename... Ts>
	ste_resource_pool(Ts&&... res_ctor_args)
		: res_params(std::forward<Ts>(res_ctor_args)...)
	{
		static_assert(std::conjunction<std::is_copy_constructible<Ts>...>::value, "One of Ts is not copy-conctructible.");
	}
	virtual ~ste_resource_pool() {}

	/**
	*	@brief	Returns an instance of T constructed with the same arguments as passed to the ctor of the pool
	*/
	auto claim() {
		auto p = pool.pop();
		if (p != nullptr) {
			// Return from pool
			resource_reclamation_policy<value_type>::reset(*p);
			return resource_t(&pool, 
							  std::move(p));
		}

		// Create new
		auto ptr = lib::allocator<T>().allocate(1);
		tuple_call(&T::template _ste_resource_pool_resource_creator<T>,
				   std::tuple_cat(std::make_tuple(ptr), res_params));
		return resource_t(&pool,
						  resource_ptr_t(ptr));
	}
};

}
}
