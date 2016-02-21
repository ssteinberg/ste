// StE
// � Shlomi Steinberg, 2015

#pragma once

namespace StE {
namespace LLR {
	
template <typename T>
class texture1D_size_type {
private:
	T size;
	
public:
	texture1D_size_type() = default;
	texture1D_size_type(const T &t) : size(t) {}
	texture1D_size_type(const texture1D_size_type &) = default;
	texture1D_size_type(texture1D_size_type &&) = default;
	texture1D_size_type &operator=(const texture1D_size_type &) = default;
	texture1D_size_type &operator=(texture1D_size_type &&) = default;
	
	operator T&() { return size; }
	operator const T&() const { return size; }
	T& operator[](int index) { assert(index == 0); return size; }
	const T& operator[](int index) const { assert(index == 0); return size; }
};
	
}
}
