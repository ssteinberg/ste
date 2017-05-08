//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <ste_resource_pool_traits.hpp>

namespace ste {
namespace gl {

template <typename T>
struct ste_resource_pool_reclamation_policy {
	static constexpr bool allow_non_const_resource = ste_resource_pool_is_resetable<T>::value;

	template <typename S = T>
	static void reset(S &t,
					  typename std::enable_if<ste_resource_pool_is_resetable<S>::value>::type* = nullptr) {
		t.reset();
	}
	template <typename S = T>
	static void reset(S &,
					  typename std::enable_if<!ste_resource_pool_is_resetable<S>::value>::type* = nullptr) {}
};

}
}
