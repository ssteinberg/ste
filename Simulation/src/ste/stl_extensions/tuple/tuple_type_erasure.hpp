// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "typelist.hpp"

#include <cstring>
#include <vector>

#include <functional>
#include <type_traits>

namespace StE {

namespace _tuple_type_erasure {

template <unsigned N, typename... Tail>
struct _type_checker_helper : public _type_checker_helper<N-1, Tail...> {
	using T = typename std::remove_cv_t<typename std::remove_reference_t<typename typelist_type_at<0, Tail...>::type>>;
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
};
template <typename... Tail>
struct _type_checker_helper<0, Tail...> {};
template <typename... Args>
struct _type_checker : public _type_checker_helper<sizeof...(Args), Args...> {};

}

class tuple_type_erasure {
private:
	std::vector<char> data;

public:
	tuple_type_erasure() = default;

	template <typename... Args>
	tuple_type_erasure(const std::tuple<Args...> &tuple_args, std::enable_if_t<1 < sizeof...(Args)>* = nullptr) : data(sizeof(tuple_args)) {
		_tuple_type_erasure::_type_checker<Args...> check;

		std::memcpy(data.data(), &tuple_args, data.size());
	}
	template <typename T>
	tuple_type_erasure(const std::tuple<T> &tuple) : data(sizeof(typename std::remove_reference_t<T>)) {
		*reinterpret_cast<typename std::remove_reference_t<T>*>(&data[0]) = std::get<0>(tuple);
	}

	tuple_type_erasure(tuple_type_erasure &&) = default;
	tuple_type_erasure(const tuple_type_erasure &) = default;
	tuple_type_erasure &operator=(tuple_type_erasure &&) = default;
	tuple_type_erasure &operator=(const tuple_type_erasure &) = default;

	bool operator==(const tuple_type_erasure &te) const {
		return data.size() == te.data.size() &&
			   std::memcmp(&data[0], &te.data[0], data.size()) == 0;
	}
	bool operator!=(const tuple_type_erasure &te) const {
		return !((*this) == te);
	}

	template <typename... Args>
	bool compare_weak(const std::tuple<Args...> &tuple_args, std::enable_if_t<1 < sizeof...(Args)>* = nullptr) const {
		_tuple_type_erasure::_type_checker<Args...> check;

		std::size_t s = sizeof(tuple_args);

		if (s == data.size())
			return std::memcmp(&tuple_args, data.data(), s) == 0;
		return false;
	}
	template <typename T>
	bool compare_weak(const std::tuple<T> &tuple) const {
		_tuple_type_erasure::_type_checker<T> check;

		using Type = typename std::remove_reference_t<T>;

		std::size_t s = sizeof(Type);

		if (s == data.size())
			return *reinterpret_cast<const Type*>(&data[0]) == std::get<0>(tuple);
		return false;
	}

	template <typename... Args>
	auto get_weak(std::enable_if_t<1 < sizeof...(Args)>* = nullptr) const {
		_tuple_type_erasure::_type_checker<Args...> check;

		using T = std::tuple<decltype(Args{})...>;
		T t;

		assert(sizeof(t) == data.size());
		auto s = std::min(sizeof(t), data.size());

		std::memcpy(&t, data.data(), s);

		return t;
	}
	template <typename T>
	auto get_weak() const {
		_tuple_type_erasure::_type_checker<T> check;

		using Type = typename std::remove_reference_t<T>;

		assert(sizeof(Type) == data.size());
		if (sizeof(Type) != data.size())
			return std::tuple<Type>();

		return std::tuple<Type>(*reinterpret_cast<const T*>(&data[0]));
	}
};

}
