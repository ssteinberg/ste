// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

namespace StE {

namespace detail {
		
template <template <typename...> class C, typename...Ts>
std::true_type is_base_of_template_impl(const C<Ts...>*);

template <template <typename...> class C>
std::false_type is_base_of_template_impl(...);

}

template <typename T, template <typename...> class C>
using is_base_of = decltype(detail::is_base_of_template_impl<C>(std::declval<T*>()));

}
