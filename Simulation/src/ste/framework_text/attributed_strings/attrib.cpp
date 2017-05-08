#include <stdafx.hpp>
#include <attrib.hpp>

#include <attributed_string_common.hpp>

using namespace ste::text;
using namespace ste::text::Attributes;

const char rgb::type_id = 0;
const char stroke::type_id = 0;
const char Attributes::font::type_id = 0;
const char size::type_id = 0;
const char line_height::type_id = 0;
const char kern::type_id = 0;
const char align::type_id = 0;
const char weight::type_id = 0;
const char underline::type_id = 0;
const char italic::type_id = 0;
const char link::type_id = 0;

attributed_string_common<char> attrib::operator()(const std::string &str) const {
	attributed_string_common<char> newstr(str);
	newstr.add_attrib({ 0,newstr.length() }, *this);
	return newstr;
}

attributed_string_common<char16_t> attrib::operator()(const std::u16string &str) const {
	attributed_string_common<char16_t> newstr(str);
	newstr.add_attrib({ 0,newstr.length() }, *this);
	return newstr;
}

attributed_string_common<char32_t> attrib::operator()(const std::u32string &str) const {
	attributed_string_common<char32_t> newstr(str);
	newstr.add_attrib({ 0,newstr.length() }, *this);
	return newstr;
}

attributed_string_common<wchar_t> attrib::operator()(const std::wstring &str) const {
	attributed_string_common<wchar_t> newstr(str);
	newstr.add_attrib({ 0,newstr.length() }, *this);
	return newstr;
}

attributed_string_common<char> attrib::operator()(const char* str) const {
	return (*this)(std::string(str));
}

attributed_string_common<char16_t> attrib::operator()(const char16_t* str) const {
	return (*this)(std::u16string(str));
}

attributed_string_common<char32_t> attrib::operator()(const char32_t* str) const {
	return (*this)(std::u32string(str));
}

attributed_string_common<wchar_t> attrib::operator()(const wchar_t* str) const {
	return (*this)(std::wstring(str));
}
