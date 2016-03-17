// StE
// � Shlomi Steinberg, 2015

#pragma once

namespace StE {
namespace Core {
	
template <typename T>
class texture1D_size_type {
public:
	T x;

	texture1D_size_type() = default;
	texture1D_size_type(const T &t) : x(t) {}
	texture1D_size_type(const texture1D_size_type &) = default;
	texture1D_size_type(texture1D_size_type &&) = default;
	texture1D_size_type &operator=(const texture1D_size_type &) = default;
	texture1D_size_type &operator=(texture1D_size_type &&) = default;
	
	operator T&() { return x; }
	operator const T&() const { return x; }
	T& operator[](int index) { assert(index == 0); return x; }
	const T& operator[](int index) const { assert(index == 0); return x; }
};
	
}
}
