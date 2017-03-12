//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <ste_resource_impl.hpp>
#include <ste_resource_type_traits.hpp>
#include <ste_resource_creation_policy.hpp>

namespace StE {

namespace _detail {

// Resource base type selector
template <typename T, class resource_deferred_policy>
struct ste_resource_resource_impl_type {
	static constexpr bool deferred = ste_resource_conforms_to_deferred_loading<T>::value;
	static constexpr bool move_constructible = std::is_move_constructible<T>::value;
	static constexpr bool move_assignable = std::is_move_assignable<T>::value;
	static constexpr bool default_constructible = std::is_default_constructible<T>::value;

	static_assert(!deferred || move_constructible, "Resource conforming to ste_resource_deferred_create_trait must be move-contructible");

	using non_deferred_value = ste_resource_wrapper<T>;
	using deferred_value = typename std::conditional<
		move_assignable && default_constructible,
		ste_resource_deferred_move_assignable_default_constructible<T, resource_deferred_policy>,
		ste_resource_deferred_ptr_wrap<T, resource_deferred_policy>
	>::type;

	using value = typename std::conditional <
		deferred,
		deferred_value,
		non_deferred_value
	>::type;
};

}

/**
*	@brief	Creates a resource wrapper for type T.
*	
*			If T is move-constructible and inherits from 'ste_resource_deferred_create_trait', T's creation will be deferred.
*			The deferring is done according to the policy 'resource_deferred_policy' which defaults to
*			'ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>'. For more information see
*			policies' documentation.
*			Parameters' lifetime is the caller's responsibility.
*			
*			Storage of the resource is inline if T is default-constructible and move-assignable, otherwise T will be wrapped 
*			in a std::unique_ptr.
*
*			ste_resource exposes a simple access interface: get() and overloaded member access opeartor->.
*			Calling getters from multiple threads is safe.
*			Getters will block, if neccessary, until resource creation is complete.
*/
template <typename T, class resource_deferred_policy = ste_resource_deferred_creation_policy_async<ste_resource_async_policy_task_scheduler>>
class ste_resource : public _detail::ste_resource_resource_impl_type<T, resource_deferred_policy>::value {
	using Base = typename _detail::ste_resource_resource_impl_type<T, resource_deferred_policy>::value;

public:
	/**
	*	@brief	Creates the resource.
	*			If T takes a context as first parameter, ctx will also be forwarded to T's ctor, along with params.
	*	
	*	@param	ctx		Context
	*	@params	params	Parameters passed to T
	*/
	template <typename ... Params>
	ste_resource(const ste_context &ctx,
				 Params&&... params)
		: Base(ctx,
			   std::forward<Params>(params)...)
	{}
	/**
	*	@brief	Creates the resource without deferring the creation.
	*
	*	@params	params	Parameters passed to T
	*/
	template <typename ... Params>
	ste_resource(const ste_resource_dont_defer &,
				 Params&&... params)
		: Base(ste_resource_dont_defer(),
			   std::forward<Params>(params)...)
	{}

	~ste_resource() noexcept {}

	// For intellisense
	T& get() & { return Base::get(); }
	T&& get() && { return std::move(Base::get()); }
	const T& get() const& { return Base::get(); }
	T* operator->() { return Base::operator->(); }
	const T* operator->() const { return Base::operator->(); }
};

}
