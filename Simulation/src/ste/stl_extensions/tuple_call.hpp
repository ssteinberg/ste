// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include <functional>
#include <utility>
#include <tuple>

namespace StE {

namespace _tuple_call_detail {
	template <typename F, typename Tuple, bool Done, int Total, int... N>
	struct call_impl {
		static void call(F f, Tuple &&t) {
			call_impl<F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, std::forward<Tuple>(t));
		}
	};

	template <typename F, typename Tuple, int Total, int... N>
	struct call_impl<F, Tuple, true, Total, N...> {
		static void call(F f, Tuple &&t) {
			f(std::get<N>(std::forward<Tuple>(t))...);
		}
	};

	template <typename B, typename F, typename Tuple, bool Done, int Total, int... N>
	struct call_impl_ex {
		static void call(B b, F f, Tuple &&t) {
			call_impl_ex<B, F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(std::forward<B>(b), f, std::forward<Tuple>(t));
		}
	};

	template <typename B, typename F, typename Tuple, int Total, int... N>
	struct call_impl_ex<B, F, Tuple, true, Total, N...> {
		static void call(B &&b, F f, Tuple &&t) {
			(b->*f)(std::get<N>(std::forward<Tuple>(t))...);
		}
	};
}

template <typename F, typename Tuple>
void tuple_call(F f, Tuple &&t) {
	using T = typename std::decay<Tuple>::type;
	_tuple_call_detail::call_impl<F, Tuple, 0 == std::tuple_size<T>::value, std::tuple_size<T>::value>::call(f, std::forward<Tuple>(t));
}

template <typename B, typename F, typename Tuple>
void tuple_call(B &&b, F f, Tuple &&t) {
	using T = typename std::decay<Tuple>::type;
	_tuple_call_detail::call_impl_ex<B, F, Tuple, 0 == std::tuple_size<T>::value, std::tuple_size<T>::value>::call(std::forward<B>(b), f, std::forward<Tuple>(t));
}

}
