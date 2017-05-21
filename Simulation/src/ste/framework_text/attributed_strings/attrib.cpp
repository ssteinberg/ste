#include <stdafx.hpp>
#include <attrib.hpp>

#include <attributed_string_common.hpp>

using namespace ste;
using namespace ste::text;
using namespace ste::text::attributes;

attributed_string_common<char> attrib::operator()(const lib::string &str) const {
	attributed_string_common<char> newstr(str);
	newstr.add_attrib({ 0, static_cast<std::uint32_t>(newstr.length()) }, *this);
	return newstr;
}

attributed_string_common<char16_t> attrib::operator()(const lib::u16string &str) const {
	attributed_string_common<char16_t> newstr(str);
	newstr.add_attrib({ 0,static_cast<std::uint32_t>(newstr.length()) }, *this);
	return newstr;
}

attributed_string_common<char32_t> attrib::operator()(const lib::u32string &str) const {
	attributed_string_common<char32_t> newstr(str);
	newstr.add_attrib({ 0,static_cast<std::uint32_t>(newstr.length()) }, *this);
	return newstr;
}

attributed_string_common<wchar_t> attrib::operator()(const lib::wstring &str) const {
	attributed_string_common<wchar_t> newstr(str);
	newstr.add_attrib({ 0,static_cast<std::uint32_t>(newstr.length()) }, *this);
	return newstr;
}

attributed_string_common<char> attrib::operator()(lib::string &&str) const {
	attributed_string_common<char> newstr(std::move(str));
	newstr.add_attrib({ 0,static_cast<std::uint32_t>(newstr.length()) }, *this);
	return newstr;
}

attributed_string_common<char16_t> attrib::operator()(lib::u16string &&str) const {
	attributed_string_common<char16_t> newstr(std::move(str));
	newstr.add_attrib({ 0,static_cast<std::uint32_t>(newstr.length()) }, *this);
	return newstr;
}

attributed_string_common<char32_t> attrib::operator()(lib::u32string &&str) const {
	attributed_string_common<char32_t> newstr(std::move(str));
	newstr.add_attrib({ 0,static_cast<std::uint32_t>(newstr.length()) }, *this);
	return newstr;
}

attributed_string_common<wchar_t> attrib::operator()(lib::wstring &&str) const {
	attributed_string_common<wchar_t> newstr(std::move(str));
	newstr.add_attrib({ 0,static_cast<std::uint32_t>(newstr.length()) }, *this);
	return newstr;
}

attributed_string_common<char> attrib::operator()(const char* str) const {
	return (*this)(lib::string(str));
}

attributed_string_common<char16_t> attrib::operator()(const char16_t* str) const {
	return (*this)(lib::u16string(str));
}

attributed_string_common<char32_t> attrib::operator()(const char32_t* str) const {
	return (*this)(lib::u32string(str));
}

attributed_string_common<wchar_t> attrib::operator()(const wchar_t* str) const {
	return (*this)(lib::wstring(str));
}


namespace ste::text::attributes {

static const rgb clear_color = rgb({ 0, 0, 0, 0 });
static const rgb maroon = rgb({ 128, 0, 0 });
static const rgb dark_red = rgb({ 139, 0, 0 });
static const rgb brown = rgb({ 165, 42, 42 });
static const rgb firebrick = rgb({ 178, 34, 34 });
static const rgb crimson = rgb({ 220, 20, 60 });
static const rgb red = rgb({ 255, 0, 0 });
static const rgb tomato = rgb({ 255, 99, 71 });
static const rgb coral = rgb({ 255, 127, 80 });
static const rgb indian_red = rgb({ 205, 92, 92 });
static const rgb light_coral = rgb({ 240, 128, 128 });
static const rgb dark_salmon = rgb({ 233, 150, 122 });
static const rgb salmon = rgb({ 250, 128, 114 });
static const rgb light_salmon = rgb({ 255, 160, 122 });
static const rgb orange_red = rgb({ 255, 69, 0 });
static const rgb dark_orange = rgb({ 255, 140, 0 });
static const rgb orange = rgb({ 255, 165, 0 });
static const rgb gold = rgb({ 255, 215, 0 });
static const rgb dark_golden_rod = rgb({ 184, 134, 11 });
static const rgb golden_rod = rgb({ 218, 165, 32 });
static const rgb pale_golden_rod = rgb({ 238, 232, 170 });
static const rgb dark_khaki = rgb({ 189, 183, 107 });
static const rgb khaki = rgb({ 240, 230, 140 });
static const rgb olive = rgb({ 128, 128, 0 });
static const rgb yellow = rgb({ 255, 255, 0 });
static const rgb yellow_green = rgb({ 154, 205, 50 });
static const rgb dark_olive_green = rgb({ 85, 107, 47 });
static const rgb olive_drab = rgb({ 107, 142, 35 });
static const rgb lawn_green = rgb({ 124, 252, 0 });
static const rgb chart_reuse = rgb({ 127, 255, 0 });
static const rgb green_yellow = rgb({ 173, 255, 47 });
static const rgb dark_green = rgb({ 0, 100, 0 });
static const rgb green = rgb({ 0, 128, 0 });
static const rgb forest_green = rgb({ 34, 139, 34 });
static const rgb lime = rgb({ 0, 255, 0 });
static const rgb lime_green = rgb({ 50, 205, 50 });
static const rgb light_green = rgb({ 144, 238, 144 });
static const rgb pale_green = rgb({ 152, 251, 152 });
static const rgb dark_sea_green = rgb({ 143, 188, 143 });
static const rgb medium_spring_green = rgb({ 0, 250, 154 });
static const rgb spring_green = rgb({ 0, 255, 127 });
static const rgb sea_green = rgb({ 46, 139, 87 });
static const rgb medium_aqua_marine = rgb({ 102, 205, 170 });
static const rgb medium_sea_green = rgb({ 60, 179, 113 });
static const rgb light_sea_green = rgb({ 32, 178, 170 });
static const rgb dark_slate_gray = rgb({ 47, 79, 79 });
static const rgb teal = rgb({ 0, 128, 128 });
static const rgb dark_cyan = rgb({ 0, 139, 139 });
static const rgb aqua = rgb({ 0, 255, 255 });
static const rgb cyan = rgb({ 0, 255, 255 });
static const rgb light_cyan = rgb({ 224, 255, 255 });
static const rgb dark_turquoise = rgb({ 0, 206, 209 });
static const rgb turquoise = rgb({ 64, 224, 208 });
static const rgb medium_turquoise = rgb({ 72, 209, 204 });
static const rgb pale_turquoise = rgb({ 175, 238, 238 });
static const rgb aqua_marine = rgb({ 127, 255, 212 });
static const rgb powder_blue = rgb({ 176, 224, 230 });
static const rgb cadet_blue = rgb({ 95, 158, 160 });
static const rgb steel_blue = rgb({ 70, 130, 180 });
static const rgb corn_flower_blue = rgb({ 100, 149, 237 });
static const rgb deep_sky_blue = rgb({ 0, 191, 255 });
static const rgb dodger_blue = rgb({ 30, 144, 255 });
static const rgb light_blue = rgb({ 173, 216, 230 });
static const rgb sky_blue = rgb({ 135, 206, 235 });
static const rgb light_sky_blue = rgb({ 135, 206, 250 });
static const rgb midnight_blue = rgb({ 25, 25, 112 });
static const rgb navy = rgb({ 0, 0, 128 });
static const rgb dark_blue = rgb({ 0, 0, 139 });
static const rgb medium_blue = rgb({ 0, 0, 205 });
static const rgb blue = rgb({ 0, 0, 255 });
static const rgb royal_blue = rgb({ 65, 105, 225 });
static const rgb blue_violet = rgb({ 138, 43, 226 });
static const rgb indigo = rgb({ 75, 0, 130 });
static const rgb dark_slate_blue = rgb({ 72, 61, 139 });
static const rgb slate_blue = rgb({ 106, 90, 205 });
static const rgb medium_slate_blue = rgb({ 123, 104, 238 });
static const rgb medium_purple = rgb({ 147, 112, 219 });
static const rgb dark_magenta = rgb({ 139, 0, 139 });
static const rgb dark_violet = rgb({ 148, 0, 211 });
static const rgb dark_orchid = rgb({ 153, 50, 204 });
static const rgb medium_orchid = rgb({ 186, 85, 211 });
static const rgb purple = rgb({ 128, 0, 128 });
static const rgb thistle = rgb({ 216, 191, 216 });
static const rgb plum = rgb({ 221, 160, 221 });
static const rgb violet = rgb({ 238, 130, 238 });
static const rgb magenta = rgb({ 255, 0, 255 });
static const rgb orchid = rgb({ 218, 112, 214 });
static const rgb medium_violet_red = rgb({ 199, 21, 133 });
static const rgb pale_violet_red = rgb({ 219, 112, 147 });
static const rgb deep_pink = rgb({ 255, 20, 147 });
static const rgb hot_pink = rgb({ 255, 105, 180 });
static const rgb light_pink = rgb({ 255, 182, 193 });
static const rgb pink = rgb({ 255, 192, 203 });
static const rgb antique_white = rgb({ 250, 235, 215 });
static const rgb beige = rgb({ 245, 245, 220 });
static const rgb bisque = rgb({ 255, 228, 196 });
static const rgb blanched_almond = rgb({ 255, 235, 205 });
static const rgb wheat = rgb({ 245, 222, 179 });
static const rgb corn_silk = rgb({ 255, 248, 220 });
static const rgb lemon_chiffon = rgb({ 255, 250, 205 });
static const rgb light_golden_rod_yellow = rgb({ 250, 250, 210 });
static const rgb light_yellow = rgb({ 255, 255, 224 });
static const rgb saddle_brown = rgb({ 139, 69, 19 });
static const rgb sienna = rgb({ 160, 82, 45 });
static const rgb chocolate = rgb({ 210, 105, 30 });
static const rgb peru = rgb({ 205, 133, 63 });
static const rgb sandy_brown = rgb({ 244, 164, 96 });
static const rgb burly_wood = rgb({ 222, 184, 135 });
static const rgb tan = rgb({ 210, 180, 140 });
static const rgb rosy_brown = rgb({ 188, 143, 143 });
static const rgb moccasin = rgb({ 255, 228, 181 });
static const rgb navajo_white = rgb({ 255, 222, 173 });
static const rgb peach_puff = rgb({ 255, 218, 185 });
static const rgb misty_rose = rgb({ 255, 228, 225 });
static const rgb lavender_blush = rgb({ 255, 240, 245 });
static const rgb linen = rgb({ 250, 240, 230 });
static const rgb old_lace = rgb({ 253, 245, 230 });
static const rgb papaya_whip = rgb({ 255, 239, 213 });
static const rgb sea_shell = rgb({ 255, 245, 238 });
static const rgb mint_cream = rgb({ 245, 255, 250 });
static const rgb slate_gray = rgb({ 112, 128, 144 });
static const rgb light_slate_gray = rgb({ 119, 136, 153 });
static const rgb light_steel_blue = rgb({ 176, 196, 222 });
static const rgb lavender = rgb({ 230, 230, 250 });
static const rgb floral_white = rgb({ 255, 250, 240 });
static const rgb alice_blue = rgb({ 240, 248, 255 });
static const rgb ghost_white = rgb({ 248, 248, 255 });
static const rgb honeydew = rgb({ 240, 255, 240 });
static const rgb ivory = rgb({ 255, 255, 240 });
static const rgb azure = rgb({ 240, 255, 255 });
static const rgb snow = rgb({ 255, 250, 250 });
static const rgb black = rgb({ 0, 0, 0 });
static const rgb dim_gray = rgb({ 105, 105, 105 });
static const rgb gray = rgb({ 128, 128, 128 });
static const rgb dark_gray = rgb({ 169, 169, 169 });
static const rgb silver = rgb({ 192, 192, 192 });
static const rgb light_gray = rgb({ 211, 211, 211 });
static const rgb gainsboro = rgb({ 220, 220, 220 });
static const rgb white_smoke = rgb({ 245, 245, 245 });
static const rgb white = rgb({ 255, 255, 255 });

static const size huge = size(96);
static const size vvlarge = size(78);
static const size vlarge = size(64);
static const size large = size(48);
static const size regular = size(36);
static const size small = size(28);
static const size vsmall = size(20);
static const size tiny = size(12);

static const align left = align(align::alignment::Left);
static const align center = align(align::alignment::Center);
static const align right = align(align::alignment::Right);

static const weight b = weight(700);

static const underline u = underline();

static const italic i = italic();

}
