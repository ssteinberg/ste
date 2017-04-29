// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <type_traits>
#include <limits>

namespace ste {

namespace _detail {

template <typename T, bool safe>
struct safe_numerical_limits_impl {
	using type = std::numeric_limits<T>;
};
template <typename T>
struct safe_numerical_limits_impl<T, false> {
	using type = std::numeric_limits<void>;
};

}

template <typename T>
struct safe_numerical_limits {
	static constexpr auto safe = std::conjunction_v<
		std::is_convertible<T, T>,
		std::negation<std::is_abstract<T>>
	>;
	using type = typename _detail::safe_numerical_limits_impl<T, safe>::type;
};
template <typename T>
using safe_numerical_limits_t = typename safe_numerical_limits<T>::type;

}
