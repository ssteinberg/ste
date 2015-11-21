// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

namespace StE {

template <typename T>
T hash_combine(const T &h1, const T &h2) { assert(false); }

template<>
std::size_t hash_combine<std::size_t>(const std::size_t &h1, const std::size_t &h2) {
	return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
}

}
