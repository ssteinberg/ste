//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <numerical_type.hpp>

namespace ste {

namespace _detail {

struct layers_unique_tag {};
struct levels_unique_tag {};

}

/*
*	@brief	Type used to represent count of layers in a texture.
*/
using layers_t = numerical_type<std::uint32_t, _detail::layers_unique_tag>;
/*
*	@brief	Type used to represent count of levels (mipmaps) in a texture.
*/
using levels_t = numerical_type<std::uint32_t, _detail::levels_unique_tag>;

inline auto operator"" _layers(unsigned long long int val) { return layers_t(static_cast<layers_t::value_type>(val)); }
inline auto operator"" _mips(unsigned long long int val) { return levels_t(static_cast<levels_t::value_type>(val)); }

}
