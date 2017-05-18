//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <string>

namespace ste {
namespace lib {

template <typename Elem, typename Traits>
using basic_string = std::basic_string<Elem, Traits, allocator<Elem>>;

using string = basic_string<char, std::char_traits<char>>;
using u16string = basic_string<char16_t, std::char_traits<char16_t>>;
using u32string = basic_string<char32_t, std::char_traits<char32_t>>;
using wstring = basic_string<wchar_t, std::char_traits<wchar_t>>;

}
}
