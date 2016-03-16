// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include <functional>
#include <utility>

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
}

template <typename F, typename Tuple>
void tuple_call(F f, Tuple &&t) {
	using T = typename std::decay<Tuple>::type;
	_tuple_call_detail::call_impl<F, Tuple, 0 == std::tuple_size<T>::value, std::tuple_size<T>::value>::call(f, std::forward<Tuple>(t));
}

}
