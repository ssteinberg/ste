// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "attributed_string.hpp"

namespace StE {
namespace Text {

using AttributedString = attributed_string<char>;
using AttributedWString = attributed_string<wchar_t>;
using AttributedU16String = attributed_string<char16_t>;
using AttributedU32String = attributed_string<char32_t>;

}
}
