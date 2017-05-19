//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <string>
#include <cstdio>
#include <type_traits>

namespace ste {
namespace lib {

template <typename Elem, typename Traits = std::char_traits<Elem>>
using basic_string = std::basic_string<Elem, Traits, allocator<Elem>>;

using string = basic_string<char, std::char_traits<char>>;
using u16string = basic_string<char16_t, std::char_traits<char16_t>>;
using u32string = basic_string<char32_t, std::char_traits<char32_t>>;
using wstring = basic_string<wchar_t, std::char_traits<wchar_t>>;

namespace _detail {

template <std::size_t sizeof_voidp>
struct _uint_to_buff_impl {};
template <>
struct _uint_to_buff_impl<4> {
	template <class Elem, class T>
	Elem* _uint_to_buff(Elem* next, T uval) {
		static_assert(std::is_unsigned_v<T>, "T must be unsigned");

#pragma warning(push)
#pragma warning(disable: 4127)	// conditional expression is constant
		if (sizeof(T) > 4) {
			while (uval > 0xFFFFFFFFU) {
				auto chunk = static_cast<std::uint32_t>(uval % 1000000000);
				uval /= 1000000000;

				for (int i = 0; i != 9; ++i) {
					*--next = '0' + chunk % 10;
					chunk /= 10;
				}
			}
		}
#pragma warning(pop)

		auto trunc = static_cast<std::uint32_t>(uval);

		do {
			*--next = '0' + trunc % 10;
			trunc /= 10;
		} while (trunc != 0);

		return next;
	}
};
template <>
struct _uint_to_buff_impl<8> {
	template <class Elem, class T>
	Elem* _uint_to_buff(Elem* next, T uval) {
		static_assert(std::is_unsigned_v<T>, "T must be unsigned");

		auto trunc = uval;

		do {
			*--next = '0' + trunc % 10;
			trunc /= 10;
		} while (trunc != 0);

		return next;
	}
};

template <class Elem, class T>
Elem* _uint_to_buff(Elem* next, T uval) {
	return _uint_to_buff_impl<sizeof(void*)>()._uint_to_buff(next, uval);
}

template <class Elem, class T>
basic_string<Elem> _int_to_string(const T val) {
	static_assert(std::is_integral_v<T>, "T must be integral");
	using U = std::make_unsigned_t<T>;
	Elem buf[21];
	Elem* const end = _STD end(buf);
	Elem* next = end;
	auto uval = static_cast<U>(val);
	if (val < 0) {
		next = _uint_to_buff(next, 0 - uval);
		*--next = '-';
	}
	else
		next = _uint_to_buff(next, uval);

	return basic_string<Elem>(next, end);
}

template <class T>
string _float_to_string(const char *str, T val) {
	static_assert(std::is_floating_point<T>::value, "T must be floating point");

	int l = ::snprintf(nullptr, 0, str, val);
	string t(l + 1, '\0');
	::snprintf(t.data(), l + 1, str, val);
	t.resize(l);
	return t;
}

template <class T>
wstring _float_to_wstring(const wchar_t *str, T val) {
	static_assert(std::is_floating_point<T>::value, "T must be floating point");

	int l = 256;
	wstring t(l + 1, L'\0');
	while (::swprintf(t.data(), l + 1, str, val) < 0) {
		l *= 2;
		t.resize(l);
	}
	t.resize(l);
	return t;
}

}

// to_string scalar types
inline string to_string(int v) {
	return _detail::_int_to_string<char>(v);
}

inline string to_string(unsigned int v) {
	return _detail::_int_to_string<char>(v);
}

inline string to_string(long v) {
	return _detail::_int_to_string<char>(v);
}

inline string to_string(unsigned long v) {
	return _detail::_int_to_string<char>(v);
}

inline string to_string(long long v) {
	return _detail::_int_to_string<char>(v);
}

inline string to_string(unsigned long long v) {
	return _detail::_int_to_string<char>(v);
}

inline string to_string(float v) {
	return _detail::_float_to_string("%f", v);
}

inline string to_string(double v) {
	return _detail::_float_to_string("%f", v);
}

inline string to_string(long double v) {
	return _detail::_float_to_string("%Lf", v);
}


// to_wstring scalar types
inline wstring to_wstring(int v) {
	return _detail::_int_to_string<wchar_t>(v);
}

inline wstring to_wstring(unsigned int v) {
	return _detail::_int_to_string<wchar_t>(v);
}

inline wstring to_wstring(long v) {
	return _detail::_int_to_string<wchar_t>(v);
}

inline wstring to_wstring(unsigned long v) {
	return _detail::_int_to_string<wchar_t>(v);
}

inline wstring to_wstring(long long v) {
	return _detail::_int_to_string<wchar_t>(v);
}

inline wstring to_wstring(unsigned long long v) {
	return _detail::_int_to_string<wchar_t>(v);
}

inline wstring to_wstring(float v) {
	return _detail::_float_to_wstring(L"%f", v);
}

inline wstring to_wstring(double v) {
	return _detail::_float_to_wstring(L"%f", v);
}

inline wstring to_wstring(long double v) {
	return _detail::_float_to_wstring(L"%Lf", v);
}


// From std::string
template <typename Elem, typename Traits, typename Allocator>
auto to_string(const std::basic_string<Elem, Traits, Allocator> &src) {
	basic_string<Elem, Traits> str;
	str.resize(src.size());
	std::copy(src.begin(), src.end(), str.begin());

	return str;
}

}
}


// Boost serialization
namespace boost::serialization {

template<class Archive, typename Elem, typename Traits>
void serialize(Archive &ar, ste::lib::basic_string<Elem, Traits> &str, const unsigned int version) {
	std::string stdstr;
	stdstr.resize(str.size());
	std::copy(str.begin(), str.end(), stdstr.begin());

	ar & stdstr;
}

}
