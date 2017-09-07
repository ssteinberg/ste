//  StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <type_traits>

namespace ste {

inline std::size_t hash_combiner(const std::size_t &h1, const std::size_t &h2) {
    // From Boost
    return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
}

namespace _detail {

template <typename T, typename... Ts>
struct hash_combine_impl {
	static auto combiner(const T& t, const Ts&... args) {
		return hash_combiner(hash_combine_impl<Ts...>::combiner(args...),
							 hash_combine_impl<T>::combiner(t));
	}
};

template <typename T>
struct hash_combine_impl<T> {
	static auto combiner(const T& t) {
		return std::hash<T>()(t);
	}
};

}

struct hash_combine {
	template <typename... Ts>
	auto operator()(const Ts&... args) {
		return _detail::hash_combine_impl<Ts...>::combiner(args...);
	}
};

}
