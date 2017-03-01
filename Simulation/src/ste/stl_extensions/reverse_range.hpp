// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <functional>

namespace StE {

namespace _detail {
template <typename T>
struct reversion_wrapper { T& iterable; };
template <typename T>
auto begin(reversion_wrapper<T> w) { return rbegin(w.iterable); }
template <typename T>
auto end(reversion_wrapper<T> w) { return rend(w.iterable); }
}

template <typename T>
_detail::reversion_wrapper<T> reverse_range(T&& iterable) { return{ std::forward<T>(iterable) }; }

}
