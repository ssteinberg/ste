// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <functional>
#include <utility>
#include <type_traits>
#include <invoke.hpp>

namespace StE {

namespace _detail {

template <class F, class Tuple, std::size_t... I>
constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>) {
	return invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
}

} // namespace detail

template <class F, class Tuple>
constexpr decltype(auto) apply(F&& f, Tuple&& t) {
	return _detail::apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
							   std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
}

}
