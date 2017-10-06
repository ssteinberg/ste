//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <cstddef>

namespace ste {

template <typename T = std::size_t>
struct range {
	using value_type = T;

	T start{ static_cast<T>(0) };
	T length{ static_cast<T>(0) };

	range() = default;
	template <typename S, typename = std::enable_if_t<std::is_constructible_v<T, S>>>
	explicit range(range<S> r) : start(T(r.start)), length(T(r.length)) {}
	range(const T &start, const T &length) : start(start), length(length) {}

	/*
	 *	@brief	Returns true if r intersects this range
	 */
	template <typename S>
	bool intersects(const range<S> &r) const {
		return
			(start < r.start + r.length) &&
			(r.start < start + length);
	}

	/*
	*	@brief	Returns true if this range contains r
	*/
	template <typename S>
	bool contains(const range<S> &r) const {
		return
			(start <= r.start) &&
			(r.start + r.length <= start + length);
	}

	/*
	*	@brief	Returns the minimal aligned range that contains this range
	*/
	auto align(const T &alignment) const {
		static_assert(std::is_integral_v<T>, "align() works only for integral ranges");

		range ret;

		auto aligned_end = ((start + length) + alignment - 1) / alignment * alignment;
		ret.start = start / alignment * alignment;
		ret.length = aligned_end - ret.start;

		return ret;
	}

	bool operator==(const range &r) const {
		return start == r.start && length == r.length;
	}

	bool operator!=(const range &r) const {
		return !(*this == r);
	}

	bool operator<(const range &r) const {
		return start < r.start;
	}

	bool operator>(const range &r) const {
		return r < *this;
	}

	template <typename S>
	range& operator/=(const S &t) {
		start /= t;
		length /= t;
		return *this;
	}

	template <typename S>
	range& operator*=(const S &t) {
		start *= t;
		length *= t;
		return *this;
	}

	template <typename S, typename = std::enable_if_t<std::is_convertible_v<T, S>>>
	explicit operator range<S>() const {
		range<S> r;
		r.start = static_cast<S>(start);
		r.length = static_cast<S>(length);

		return r;
	}
};

template <typename T, typename S>
bool operator==(const range<T> &lhs, const range<S> &rhs) {
	return lhs.start == rhs.start && lhs.length == rhs.length;
}

template <typename T, typename S>
bool operator!=(const range<T> &lhs, const range<S> &rhs) {
	return !(lhs == rhs);
}

template <typename T, typename S>
auto operator/(const range<T> &lhs, const S &t) {
	range<T> ret;
	ret.start = lhs.start / t;
	ret.length = lhs.length / t;
	return ret;
}

template <typename T, typename S>
auto operator*(const range<T> &lhs, const S &t) {
	range<T> ret;
	ret.start = lhs.start * t;
	ret.length = lhs.length * t;
	return ret;
}

template <typename T, typename S>
auto operator*(const S &t, const range<T> &rhs) {
	range<T> ret;
	ret.start = t * rhs.start;
	ret.length = t * rhs.length;
	return ret;
}

}
