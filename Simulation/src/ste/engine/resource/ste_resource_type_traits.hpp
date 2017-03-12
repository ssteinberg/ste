//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <ste_resource_traits.hpp>

#include <type_traits>

namespace StE {

template <typename T>
struct ste_resource_conforms_to_async_load {
	static constexpr bool value = std::is_base_of<ste_resource_load_async_trait, T>::value;
};

}
