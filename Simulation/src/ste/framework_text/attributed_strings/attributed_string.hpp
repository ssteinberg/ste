// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <attributed_string_common.hpp>

namespace ste {
namespace text {

using attributed_string = attributed_string_common<char>;
using attributed_wstring = attributed_string_common<wchar_t>;
using attributed_u16string = attributed_string_common<char16_t>;
using attributed_u32string = attributed_string_common<char32_t>;

}
}
