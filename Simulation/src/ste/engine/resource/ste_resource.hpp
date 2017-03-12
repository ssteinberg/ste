//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <ste_resource_impl.hpp>

namespace StE {

/**
*	@brief	Creates a resource wrapper for type T.
*			If T is move-constructible and inherits from 'ste_resource_load_async_trait', T's creation will be deferred
*			to a background worker using the context's task scheduler. Furthermore, if T is not default-constructible or 
*			move-assignable then T will also be wrapped in a std::unique_ptr.
*			
*			ste_resource exposes simple access interface: get() and overloaded member access opeartor->.
*			Getters will block, if neccessary, until resource creation is complete.
*/
template <typename T>
class ste_resource : public _detail::ste_resource_resource_impl_type<T>::value {
	using Base = typename _detail::ste_resource_resource_impl_type<T>::value;

public:
	using Base::Base;
	template <typename ... Params>
	ste_resource(const ste_context &ctx,
				 Params&&... params) : Base(ctx, std::forward<Params>(params)...) {}
	~ste_resource() noexcept {}
};

}
