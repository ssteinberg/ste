// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

namespace StE {
namespace Core {
	
template <typename T>
class texture_1d_size_type {
public:
	T x;

	texture_1d_size_type() = default;
	texture_1d_size_type(const T &t) : x(t) {}
	texture_1d_size_type(const texture_1d_size_type &) = default;
	texture_1d_size_type(texture_1d_size_type &&) = default;
	texture_1d_size_type &operator=(const texture_1d_size_type &) = default;
	texture_1d_size_type &operator=(texture_1d_size_type &&) = default;
	
	operator T&() { return x; }
	operator const T&() const { return x; }
	T& operator[](int index) { assert(index == 0); return x; }
	const T& operator[](int index) const { assert(index == 0); return x; }
};
	
}
}
