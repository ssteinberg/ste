// StE
// © Shlomi Steinberg, 2015

#pragma once

namespace StE {

template<int N, typename H, typename... Tail>
struct typelist_type_at : typelist_type_at<N - 1, Tail...> {};
template<typename H, typename... Tail>
struct typelist_type_at<0, H, Tail...> { using type = H; };

}
