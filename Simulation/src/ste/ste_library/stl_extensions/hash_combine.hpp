// StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {

template <typename T>
T hash_combine(const T &h1, const T &h2) { static_assert(false, "Unimplemented"); }

template<>
std::size_t inline hash_combine<std::size_t>(const std::size_t &h1, const std::size_t &h2) {
	// From Boost
	return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
}

}
