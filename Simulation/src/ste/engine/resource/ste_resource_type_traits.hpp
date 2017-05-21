//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <ste_resource_traits.hpp>

#include <type_traits>

namespace ste {

template <typename T>
struct ste_resource_conforms_to_deferred_loading {
	static constexpr bool value = std::is_base_of<ste_resource_deferred_create_trait, T>::value;
};

}
