// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include <cstring>
#include <functional>
#include <vector>

namespace StE {
	
namespace _tuple_type_erasure {
	template <unsigned N, typename T, typename... Tail>
	struct _type_checker_helper : public _type_checker_helper<N-1, Tail...> {
		static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
	};
	template <typename T, typename... Tail>
	struct _type_checker_helper<0, T, Tail...> {};
	template <typename... Args>
	struct _type_checker : public _type_checker_helper<sizeof...(Args), Args...> {};
}
	
class tuple_type_erasure {	
private:
	std::size_t size;
	std::vector<char> data;

public:
	tuple_type_erasure() = default;
	
	template <typename... Args>
	tuple_type_erasure(std::tuple<Args...> &&tuple_args) : size(sizeof(tuple_args)), data(size) {
		_tuple_type_erasure::_type_checker<Args...>;

		std::memcpy(data.data(), &tuple_args, size);
	}
	template <typename... Args>
	tuple_type_erasure(Args... args) : tuple_type_erasure(std::make_tuple{ args... }) {}
	
	tuple_type_erasure(tuple_type_erasure &&) = default;
	tuple_type_erasure(const tuple_type_erasure &) = default;
	tuple_type_erasure &operator=(tuple_type_erasure &&) = default;
	tuple_type_erasure &operator=(const tuple_type_erasure &) = default;

	template <typename... Args>
	bool compare_weak(std::tuple<Args...> &&tuple_args) const {
		_tuple_type_erasure::_type_checker<Args...>;
		
		std::size_t s = sizeof(tuple_args);
		
		if (s == size)
			return std::memcmp(&tuple_args, data.data(), s);
		return false;
	}
	template <typename... Args>
	bool compare_weak(Args... args) const {
		return compare_weak(std::make_tuple{ args... });
	}
	
	template <typename... Args>
	auto get_weak() const {
		_tuple_type_erasure::_type_checker<Args...>;
		
		using T = std::tuple<decltype(Args{})...>;
		T t;
		
		assert(sizeof(t) == size);
		size = std::min(sizeof(t), size);
		
		std::memcpy(&t, data.data(), size);
		
		return t;
	}
};
	
}
