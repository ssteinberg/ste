//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <ste_resource_pool_traits.hpp>
#include <type_traits>

namespace StE {
namespace GL {

template <typename T>
struct ste_resource_pool_is_resetable { static constexpr bool value = std::is_base_of<ste_resource_pool_resetable_trait, T>::value; };

template <typename T>
struct ste_resource_pool_reclamation_policy {
	static constexpr bool allow_non_const_resource = ste_resource_pool_is_resetable<T>::value;

	template <typename S = T>
	static void reset(S &t,
					  typename std::enable_if<ste_resource_pool_is_resetable<S>::value>::type* = nullptr) {
		t.reset();
	}
	template <typename S = T>
	static void reset(S &t,
					  typename std::enable_if<!ste_resource_pool_is_resetable<S>::value>::type* = nullptr) {}
};

}
}
