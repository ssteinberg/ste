// StE
// © Shlomi Steinberg, 2015

#pragma once

namespace ste {

template<int N, typename H, typename... Tail>
struct typelist_type_at : typelist_type_at<N - 1, Tail...> {};
template<typename H, typename... Tail>
struct typelist_type_at<0, H, Tail...> { using type = H; };
template<typename H>
struct typelist_type_at<0, H> { using type = H; };

template<int N, typename... Ts>
using typelist_type_at_t = typename typelist_type_at<N, Ts...>::type;

}
