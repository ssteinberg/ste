// StE
// © Shlomi Steinberg, 2015

#pragma once

namespace StE {

struct none_t {};
constexpr none_t none = none_t();

template <typename T>
struct is_none {
	static constexpr bool value = std::is_same_v<std::remove_cv_t<T>, none_t>;
};
template <typename T>
static constexpr bool is_none_v = is_none<T>::value;

}
