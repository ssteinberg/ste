//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <cstdint>
#include <cstddef>

namespace ste {

/*
*	@brief	Type used to represent digital information size units, i.e. bytes.
*/
class ste_byte_unit_type {
public:
	using value_type = std::size_t;

protected:
	value_type val;

private:
	template <int DstExp, int SrcExp>
	static constexpr auto _multiplier() {
		return glm::exp2<double>(SrcExp - DstExp);
	}
	template <int DstExp, int SrcExp, typename T>
	static constexpr value_type _convert(T src) {
		auto v = static_cast<double>(src) * _multiplier<DstExp, SrcExp>();
		return static_cast<value_type>(glm::ceil(v));
	}

public:
	template <int SrcExp, typename T>
	static constexpr auto convert_to_byte(T src) {
		return _convert<0, SrcExp>(src);
	}

public:
	constexpr ste_byte_unit_type() = default;
	explicit constexpr ste_byte_unit_type(value_type v) noexcept : val(v) {}

	ste_byte_unit_type(ste_byte_unit_type&&) = default;
	ste_byte_unit_type(const ste_byte_unit_type&) = default;

	constexpr ste_byte_unit_type &operator=(ste_byte_unit_type &&rhs) noexcept {
		val = std::move(rhs.val);
		return *this;
	}
	constexpr ste_byte_unit_type& operator=(const ste_byte_unit_type &rhs) noexcept {
		val = rhs.val;
		return *this;
	}
	constexpr ste_byte_unit_type& operator+=(ste_byte_unit_type rhs) noexcept {
		val += rhs.val;
		return *this;
	}
	constexpr ste_byte_unit_type& operator-=(ste_byte_unit_type rhs) noexcept {
		val -= rhs.val;
		return *this;
	}
	constexpr ste_byte_unit_type& operator*=(value_type rhs) noexcept {
		val *= rhs;
		return *this;
	}
	constexpr ste_byte_unit_type& operator/=(value_type rhs) noexcept {
		val /= rhs;
		return *this;
	}

	constexpr auto& operator%=(value_type rhs) noexcept {
		val %= rhs;
		return *this;
	}
	constexpr auto& operator&=(value_type rhs) noexcept {
		val &= rhs;
		return *this;
	}
	constexpr auto& operator|=(value_type rhs) noexcept {
		val |= rhs;
		return *this;
	}
	constexpr auto& operator^=(value_type rhs) noexcept {
		val ^= rhs;
		return *this;
	}
	constexpr auto& operator<<=(value_type rhs) noexcept {
		val <<= rhs;
		return *this;
	}
	constexpr auto& operator>>=(value_type rhs) noexcept {
		val >>= rhs;
		return *this;
	}

	constexpr ste_byte_unit_type& operator++() noexcept { ++val; return *this; }
	constexpr ste_byte_unit_type& operator--() noexcept { --val; return *this; }
	constexpr ste_byte_unit_type operator++(int) noexcept { return ste_byte_unit_type(val++); }
	constexpr ste_byte_unit_type operator--(int) noexcept { return ste_byte_unit_type(val--); }

	constexpr bool operator==(ste_byte_unit_type rhs) const noexcept { return val == rhs.val; }
	constexpr bool operator!=(ste_byte_unit_type rhs) const noexcept { return val != rhs.val; }
	constexpr auto operator!() const noexcept { return !val; }
	constexpr auto operator~() const noexcept { return ste_byte_unit_type(~val); }

	explicit constexpr operator value_type() const noexcept { return val; }
};

constexpr auto operator+(ste_byte_unit_type lhs, ste_byte_unit_type rhs) noexcept { return lhs += rhs; }
constexpr auto operator-(ste_byte_unit_type lhs, ste_byte_unit_type rhs) noexcept { return lhs -= rhs; }
constexpr auto operator*(ste_byte_unit_type lhs, ste_byte_unit_type::value_type rhs) noexcept { return lhs *= rhs; }
constexpr auto operator/(ste_byte_unit_type lhs, ste_byte_unit_type::value_type rhs) noexcept { return lhs /= rhs; }

constexpr auto operator%(ste_byte_unit_type lhs, ste_byte_unit_type::value_type rhs) noexcept { return lhs %= rhs; }
constexpr auto operator&(ste_byte_unit_type lhs, ste_byte_unit_type::value_type rhs) noexcept { return lhs &= rhs; }
constexpr auto operator|(ste_byte_unit_type lhs, ste_byte_unit_type::value_type rhs) noexcept { return lhs |= rhs; }
constexpr auto operator^(ste_byte_unit_type lhs, ste_byte_unit_type::value_type rhs) noexcept { return lhs ^= rhs; }
constexpr auto operator<<(ste_byte_unit_type lhs, ste_byte_unit_type::value_type rhs) noexcept { return lhs <<= rhs; }
constexpr auto operator>>(ste_byte_unit_type lhs, ste_byte_unit_type::value_type rhs) noexcept { return lhs >>= rhs; }

using byte = ste_byte_unit_type;

inline auto operator"" _B(unsigned long long int val) { return byte(static_cast<byte::value_type>(val)); }
inline auto operator"" _kB(long double val) {
	return byte(ste_byte_unit_type::convert_to_byte<10>(val));
}
inline auto operator"" _kB(unsigned long long int val) { return byte(ste_byte_unit_type::convert_to_byte<10>(val)); }
inline auto operator"" _MB(long double val) {
	return byte(ste_byte_unit_type::convert_to_byte<20>(val));
}
inline auto operator"" _MB(unsigned long long int val) { return byte(ste_byte_unit_type::convert_to_byte<20>(val)); }
inline auto operator"" _GB(long double val) {
	return byte(ste_byte_unit_type::convert_to_byte<30>(val));
}
inline auto operator"" _GB(unsigned long long int val) { return byte(ste_byte_unit_type::convert_to_byte<30>(val)); }
inline auto operator"" _TB(long double val) {
	return byte(ste_byte_unit_type::convert_to_byte<40>(val));
}
inline auto operator"" _TB(unsigned long long int val) { return byte(ste_byte_unit_type::convert_to_byte<40>(val)); }
inline auto operator"" _PB(long double val) {
	return byte(ste_byte_unit_type::convert_to_byte<50>(val));
}
inline auto operator"" _PB(unsigned long long int val) { return byte(ste_byte_unit_type::convert_to_byte<50>(val)); }

}
