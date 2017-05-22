// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <type_traits>

namespace ste {

template <typename T>
using is_anchored = std::conditional_t<!std::is_move_constructible_v<T> && !std::is_move_assignable_v<T>, std::true_type, std::false_type>;

template <typename T>
static constexpr bool is_anchored_v = is_anchored<T>::value;

}
