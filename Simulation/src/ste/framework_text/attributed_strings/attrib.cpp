#include "stdafx.h"
#include "attrib.h"

#include "attributed_string.h"

using namespace StE::Text;
using namespace StE::Text::Attributes;

attributed_string<char> attrib::operator()(const std::string &str) const {
	attributed_string<char> newstr(str);
	newstr.add_attrib({ 0,newstr.length() }, *this);
	return newstr;
}

attributed_string<char16_t> attrib::operator()(const std::u16string &str) const {
	attributed_string<char16_t> newstr(str);
	newstr.add_attrib({ 0,newstr.length() }, *this);
	return newstr;
}

attributed_string<char32_t> attrib::operator()(const std::u32string &str) const {
	attributed_string<char32_t> newstr(str);
	newstr.add_attrib({ 0,newstr.length() }, *this);
	return newstr;
}

attributed_string<wchar_t> attrib::operator()(const std::wstring &str) const {
	attributed_string<wchar_t> newstr(str);
	newstr.add_attrib({ 0,newstr.length() }, *this);
	return newstr;
}

attributed_string<char> attrib::operator()(const char* str) const {
	return (*this)(std::string(str));
}

attributed_string<char16_t> attrib::operator()(const char16_t* str) const {
	return (*this)(std::u16string(str));
}

attributed_string<char32_t> attrib::operator()(const char32_t* str) const {
	return (*this)(std::u32string(str));
}

attributed_string<wchar_t> attrib::operator()(const wchar_t* str) const {
	return (*this)(std::wstring(str));
}
