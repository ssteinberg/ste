// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"

namespace StE {

template <typename T>
struct function_traits {
private:
	using call_type = function_traits<decltype(&T::operator())>;

	template <std::size_t N, bool in_range>
	struct arg_impl {};
	template <std::size_t N>
	struct arg_impl<N, true> { using t = typename call_type::template arg<N + 1>::t; };

public:
	using result_t = typename call_type::result_t;

	static constexpr std::size_t arity = call_type::arity - 1;

	template <std::size_t N>
	struct arg : public arg_impl<N, N < arity + 1> {
		static_assert(N < arity + 1, "N >= sizeof...(args)");
	};
};

template <typename ReturnType, typename... Args>
struct function_traits<ReturnType(Args...)> {
private:
	template <std::size_t N, bool in_range>
	struct arg_impl {};
	template <std::size_t N>
	struct arg_impl<N, true> { using t = std::tuple_element_t<N, std::tuple<Args...>>; };

public:
	static constexpr std::size_t arity = sizeof...(Args);

	using result_t = ReturnType;

	template <std::size_t N>
	struct arg : public arg_impl < N, N < arity> {
		static_assert(N < arity, "N >= sizeof...(args)"); 
	};
};

template <typename ReturnType, typename... Args>
struct function_traits<ReturnType(*)(Args...)> : public function_traits<ReturnType(Args...)> {};

template<class C, class R, class... Args>
struct function_traits<R(C::*)(Args...)> : public function_traits<R(C&, Args...)> {};

template<class C, class R, class... Args>
struct function_traits<R(C::*)(Args...) const> : public function_traits<R(C&, Args...)> {};

template<class C, class R>
struct function_traits<R(C::*)> : public function_traits<R(C&)> {};

template <typename T>
struct function_traits<T&> : public function_traits<T> {};
template <typename T>
struct function_traits<T&&> : public function_traits<T> {};

}
