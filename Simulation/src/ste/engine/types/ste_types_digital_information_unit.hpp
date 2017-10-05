//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <numerical_type.hpp>

namespace ste {

namespace _detail {

struct byte_unique_tag {};

struct byte_convertor {
	template <int Exp>
	static constexpr auto _multiplier() {
		return static_cast<double>(1ull << Exp);
	}
	template <int Exp, typename T>
	static constexpr std::size_t _convert(T src) {
		// Normalize
		auto v = static_cast<double>(src) * _multiplier<Exp>();
		// Ceil
		if (static_cast<double>(static_cast<std::size_t>(v)) == v)
			return static_cast<std::size_t>(v);
		return static_cast<std::size_t>(v) + (v > 0 ? 1 : 0);
	}
	template <int Exp, typename T>
	static constexpr auto convert_to_byte(T src) {
		return _convert<Exp>(src);
	}
};

}

/*
*	@brief	Type used to represent digital information size units, i.e. bytes.
*/
using byte_t = numerical_type<std::size_t, _detail::byte_unique_tag>;

constexpr auto operator"" _B(unsigned long long int val) { return byte_t(static_cast<byte_t::value_type>(val)); }
constexpr auto operator"" _kB(long double val) {
	return byte_t(_detail::byte_convertor::convert_to_byte<10>(val));
}
constexpr auto operator"" _kB(unsigned long long int val) { return byte_t(_detail::byte_convertor::convert_to_byte<10>(val)); }
constexpr auto operator"" _MB(long double val) {
	return byte_t(_detail::byte_convertor::convert_to_byte<20>(val));
}
constexpr auto operator"" _MB(unsigned long long int val) { return byte_t(_detail::byte_convertor::convert_to_byte<20>(val)); }
constexpr auto operator"" _GB(long double val) {
	return byte_t(_detail::byte_convertor::convert_to_byte<30>(val));
}
constexpr auto operator"" _GB(unsigned long long int val) { return byte_t(_detail::byte_convertor::convert_to_byte<30>(val)); }
constexpr auto operator"" _TB(long double val) {
	return byte_t(_detail::byte_convertor::convert_to_byte<40>(val));
}
constexpr auto operator"" _TB(unsigned long long int val) { return byte_t(_detail::byte_convertor::convert_to_byte<40>(val)); }
constexpr auto operator"" _PB(long double val) {
	return byte_t(_detail::byte_convertor::convert_to_byte<50>(val));
}
constexpr auto operator"" _PB(unsigned long long int val) { return byte_t(_detail::byte_convertor::convert_to_byte<50>(val)); }

}
