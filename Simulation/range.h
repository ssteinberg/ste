// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <cstddef>

namespace StE {

template<typename T = std::size_t>
struct range {
	T start, length;

	template<typename S>
	bool overlaps(const range<S> &r) {
		return  (start < r.start + r.length) &&
			(r.start < start + length);
	}

	template<typename S>
	bool operator==(const range<S> &r) {
		return start == r.start && length == r.length;
	}
	template<typename S>
	bool operator!=(const range<S> &r) {
		return !(*this == r);
	}
	template<typename S>
	bool operator<(const range<S> &r) {
		return start<r.start;
	}
};

template <typename T, typename S>
bool inline operator==(const range<T> &lhs, const range<S> &rhs) {
	return lhs.start == rhs.start && lhs.length == rhs.length;
}
template <typename T, typename S>
bool inline operator!=(const range<T> &lhs, const range<S> &rhs) {
	return !(lhs == rhs);
}

}
