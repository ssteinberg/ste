//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <tuple>
#include <is_base_of.hpp>

#include <lib/unique_ptr.hpp>

namespace ste {
namespace gl {

namespace _detail {

template <typename... CtorArgs>
struct ste_resource_pool_poolable_trait {
	using _ste_pool_resource_ctor_args_t = std::tuple<CtorArgs...>;
	template <typename T>
	static auto _ste_resource_pool_resource_creator(const CtorArgs&... ts) {
		return lib::allocate_unique<T>(static_cast<CtorArgs>(ts)...);
	}
};

}

/**
*	@brief	Const-resource trait for resource pools.
*			Resource pools of types conforming to ste_resource_pool_const_trait will always instantiate const resources
*			to prevent mutation.
*/
template <typename... CtorArgs>
class ste_resource_pool_const_trait : public _detail::ste_resource_pool_poolable_trait<CtorArgs...> {
public:
	virtual ~ste_resource_pool_const_trait() noexcept {}
};

/**
*	@brief	Resetable-resource trait for resource pools.
*			Resource pools of types conforming to ste_resource_pool_resetable_trait will be instantiated non-const. On release
*			to pool resources will be reset.
*			
*			reset() must be implemented in a way that resets the resource to the initial state.
*/
template <typename... CtorArgs>
class ste_resource_pool_resetable_trait : public _detail::ste_resource_pool_poolable_trait<CtorArgs...> {
public:
	virtual ~ste_resource_pool_resetable_trait() noexcept {}
	virtual void reset() = 0;
};

template <typename T>
struct ste_resource_pool_is_poolable {
	static constexpr bool value = is_base_of<T, _detail::ste_resource_pool_poolable_trait>::value;
};

template <typename T>
struct ste_resource_pool_is_resetable {
	static constexpr bool value = is_base_of<T, ste_resource_pool_resetable_trait>::value;
};

template <typename T>
struct ste_resource_pool_ctor_args_capture {
	using type = typename T::_ste_pool_resource_ctor_args_t;
};

}
}
