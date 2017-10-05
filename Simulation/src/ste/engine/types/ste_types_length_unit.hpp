//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <cstdint>
#include <cstddef>

namespace ste {

/*
 *	@brief	Type used to represent common length, area and volume units.
 */
template <int Exp, int Power = 1>
class ste_length_type {
	// Limit to length, area and volume
	static constexpr int maximal_power = 3;
	static_assert(Power >= 1 && Power <= maximal_power);

public:
	using value_type = float;

	static constexpr auto power = Power;
	static constexpr auto exponent = Exp;

protected:
	value_type val;

private:
	template <int DstExp, int SrcExp>
	static constexpr auto _multiplier() {
		return glm::pow(10.f, static_cast<value_type>(SrcExp - DstExp));
	}
	template <int DstExp, int SrcExp>
	static constexpr auto _convert(value_type src) {
		return src * _multiplier<DstExp, SrcExp>();
	}

public:
	constexpr ste_length_type() = default;
	explicit constexpr ste_length_type(value_type v) noexcept : val(v) {}
	template <int SrcExp>
	constexpr ste_length_type(ste_length_type<SrcExp, Power> src) noexcept : val(_convert<Exp, SrcExp>(static_cast<value_type>(src))) {}

	ste_length_type(ste_length_type&&) = default;
	ste_length_type(const ste_length_type&) = default;
	ste_length_type &operator=(ste_length_type&&) = default;
	ste_length_type &operator=(const ste_length_type&) = default;

	template <int E>
	constexpr ste_length_type &operator=(ste_length_type<E, Power> &&rhs) noexcept {
		val = ste_length_type<Exp, Power>(std::move(rhs)).val;
		return *this;
	}

	template <int E>
	constexpr ste_length_type& operator=(const ste_length_type<E, Power> &rhs) noexcept {
		val = ste_length_type<Exp, Power>(rhs).val;
		return *this;
	}
	template <int E>
	constexpr ste_length_type& operator+=(ste_length_type<E, Power> rhs) noexcept {
		val += ste_length_type<Exp, Power>(rhs).val;
		return *this;
	}
	template <int E>
	constexpr ste_length_type& operator-=(ste_length_type<E, Power> rhs) noexcept {
		val -= ste_length_type<Exp, Power>(rhs).val;
		return *this;
	}
	constexpr ste_length_type& operator*=(value_type rhs) noexcept {
		val *= rhs;
		return *this;
	}
	constexpr ste_length_type& operator/=(value_type rhs) noexcept {
		val /= rhs;
		return *this;
	}
	constexpr ste_length_type& operator++() noexcept { ++val; return *this; }
	constexpr ste_length_type& operator--() noexcept { --val; return *this; }
	constexpr ste_length_type operator++(int) noexcept { return ste_length_type(val++); }
	constexpr ste_length_type operator--(int) noexcept { return ste_length_type(val--); }

	template <int E>
	constexpr bool operator==(ste_length_type<E, Power> rhs) const noexcept { return val == ste_length_type<Exp, Power>(rhs).val; }
	template <int E>
	constexpr bool operator!=(ste_length_type<E, Power> rhs) const noexcept { return val != ste_length_type<Exp, Power>(rhs).val; }

	constexpr ste_length_type operator+() const noexcept { return ste_length_type(+val); }
	constexpr ste_length_type operator-() const noexcept { return ste_length_type(-val); }
	constexpr auto operator!() const noexcept { return !val; }

	explicit constexpr operator value_type() const noexcept { return val; }
	template <int DstExp>
	constexpr operator ste_length_type<DstExp, Power>() const noexcept { return _convert<DstExp, Exp>(val); }
};

template <int E1, int E2, int Power>
constexpr auto operator+(ste_length_type<E1, Power> lhs, ste_length_type<E2, Power> rhs) noexcept {
	if constexpr (E1 < E2)
		return lhs += ste_length_type<E1, Power>(rhs);
	else
		return ste_length_type<E2, Power>(lhs) += rhs;
}
template <int E1, int E2, int Power>
constexpr auto operator-(ste_length_type<E1, Power> lhs, ste_length_type<E2, Power> rhs) noexcept {
	if constexpr (E1 < E2)
		return lhs -= ste_length_type<E1, Power>(rhs);
	else
		return ste_length_type<E2, Power>(lhs) -= rhs;
}
template <int E, int Power>
constexpr auto operator*(ste_length_type<E, Power> lhs, typename ste_length_type<E, Power>::value_type rhs) noexcept { return lhs *= rhs; }
template <int E, int Power>
constexpr auto operator/(ste_length_type<E, Power> lhs, typename ste_length_type<E, Power>::value_type rhs) noexcept { return lhs /= rhs; }

/*
*	@brief	Multiplication of a length variable by length variable creates an area variable, length variable by area variable creates a volume variable, etc.
*/
template <
	int Exp1, int Exp2,
	int P1, int P2,
	typename = std::enable_if_t<P1 + P2 <= 3>
>
constexpr auto operator*(ste_length_type<Exp1, P1> lhs,
						 ste_length_type<Exp2, P2> rhs) noexcept {
	ste_length_type<Exp1 + Exp2, P1 + P2> result(static_cast<typename ste_length_type<Exp1, P1>::value_type>(lhs) * static_cast<typename ste_length_type<Exp2, P2>::value_type>(rhs));
	return result;
}

using micrometre = ste_length_type<-6>;
using millimetre = ste_length_type<-3>;
using centimetre = ste_length_type<-2>;
using decimetre = ste_length_type<-1>;
using metre = ste_length_type<0>;
using kilometre = ste_length_type<3>;

using square_millimetre = ste_length_type<-6, 2>;
using square_centimetre = ste_length_type<-4, 2>;
using square_decimetre = ste_length_type<-2, 2>;
using square_metre = ste_length_type<0, 2>;
using square_kilometre = ste_length_type<6, 2>;

using cubic_millimetre = ste_length_type<-9, 3>;
using cubic_centimetre = ste_length_type<-6, 3>;
using cubic_decimetre = ste_length_type<-3, 3>;
using cubic_metre = ste_length_type<0, 3>;
using cubic_kilometre = ste_length_type<9, 3>;

inline auto operator"" _mm(long double val) { return millimetre(static_cast<millimetre::value_type>(val)); }
inline auto operator"" _mm(unsigned long long int val) { return millimetre(static_cast<millimetre::value_type>(val)); }
inline auto operator"" _cm(long double val) { return centimetre(static_cast<centimetre::value_type>(val)); }
inline auto operator"" _cm(unsigned long long int val) { return centimetre(static_cast<centimetre::value_type>(val)); }
inline auto operator"" _m(long double val) { return metre(static_cast<metre::value_type>(val)); }
inline auto operator"" _m(unsigned long long int val) { return metre(static_cast<metre::value_type>(val)); }
inline auto operator"" _km(long double val) { return kilometre(static_cast<kilometre::value_type>(val)); }
inline auto operator"" _km(unsigned long long int val) { return kilometre(static_cast<kilometre::value_type>(val)); }

inline auto operator"" _mm2(long double val) { return square_millimetre(static_cast<square_millimetre::value_type>(val)); }
inline auto operator"" _mm2(unsigned long long int val) { return square_millimetre(static_cast<square_millimetre::value_type>(val)); }
inline auto operator"" _cm2(long double val) { return square_centimetre(static_cast<square_centimetre::value_type>(val)); }
inline auto operator"" _cm2(unsigned long long int val) { return square_centimetre(static_cast<square_centimetre::value_type>(val)); }
inline auto operator"" _m2(long double val) { return square_metre(static_cast<square_metre::value_type>(val)); }
inline auto operator"" _m2(unsigned long long int val) { return square_metre(static_cast<square_metre::value_type>(val)); }
inline auto operator"" _km2(long double val) { return square_kilometre(static_cast<square_kilometre::value_type>(val)); }
inline auto operator"" _km2(unsigned long long int val) { return square_kilometre(static_cast<square_kilometre::value_type>(val)); }

inline auto operator"" _mm3(long double val) { return cubic_millimetre(static_cast<cubic_millimetre::value_type>(val)); }
inline auto operator"" _mm3(unsigned long long int val) { return cubic_millimetre(static_cast<cubic_millimetre::value_type>(val)); }
inline auto operator"" _cm3(long double val) { return cubic_centimetre(static_cast<cubic_centimetre::value_type>(val)); }
inline auto operator"" _cm3(unsigned long long int val) { return cubic_centimetre(static_cast<cubic_centimetre::value_type>(val)); }
inline auto operator"" _m3(long double val) { return cubic_metre(static_cast<cubic_metre::value_type>(val)); }
inline auto operator"" _m3(unsigned long long int val) { return cubic_metre(static_cast<cubic_metre::value_type>(val)); }
inline auto operator"" _km3(long double val) { return cubic_kilometre(static_cast<cubic_kilometre::value_type>(val)); }
inline auto operator"" _km3(unsigned long long int val) { return cubic_kilometre(static_cast<cubic_kilometre::value_type>(val)); }

class metre_vec2 {
public:
	using T = metre;
	using value_type = T::value_type;
	using vec_type = glm::tvec2<value_type>;

	static constexpr auto length = 2;

public:
	T x;
	T y;

public:
	metre_vec2() = default;
	constexpr metre_vec2(T x, T y) noexcept : x(x), y(y) {}
	template <int Exp1, int Exp2>
	constexpr metre_vec2(ste_length_type<Exp1> x, ste_length_type<Exp2> y) noexcept : x(x), y(y) {}
	explicit constexpr metre_vec2(vec_type v) noexcept : x(T(v.x)), y(T(v.y)) {}

	metre_vec2(metre_vec2&&) = default;
	metre_vec2(const metre_vec2&) = default;

	constexpr auto& v() { return *reinterpret_cast<vec_type*>(this); }
	constexpr const auto& v() const { return *reinterpret_cast<const vec_type*>(this); }

	constexpr metre_vec2 &operator=(metre_vec2 &&rhs) noexcept {
		v() = std::move(rhs.v());
		return *this;
	}

	metre_vec2& operator=(const metre_vec2 &rhs) noexcept {
		v() = rhs.v();
		return *this;
	}
	metre_vec2& operator+=(metre_vec2 rhs) noexcept {
		v() += rhs.v();
		return *this;
	}
	metre_vec2& operator-=(metre_vec2 rhs) noexcept {
		v() -= rhs.v();
		return *this;
	}
	metre_vec2& operator*=(value_type rhs) noexcept {
		v() *= rhs;
		return *this;
	}
	metre_vec2& operator/=(value_type rhs) noexcept {
		v() /= rhs;
		return *this;
	}
	metre_vec2& operator*=(vec_type rhs) noexcept {
		v() *= rhs;
		return *this;
	}
	metre_vec2& operator/=(vec_type rhs) noexcept {
		v() /= rhs;
		return *this;
	}
	bool operator==(metre_vec2 rhs) const noexcept { return v() == rhs.v(); }
	bool operator!=(metre_vec2 rhs) const noexcept { return v() != rhs.v(); }
	metre_vec2& operator++() noexcept { ++v(); return *this; }
	metre_vec2& operator--() noexcept { --v(); return *this; }
	metre_vec2 operator++(int) noexcept { return metre_vec2(v()++); }
	metre_vec2 operator--(int) noexcept { return metre_vec2(v()--); }
	metre_vec2 operator+() const noexcept { return metre_vec2(+v()); }
	metre_vec2 operator-() const noexcept { return metre_vec2(-v()); }

	friend metre_vec2 operator+(metre_vec2 lhs, metre_vec2 rhs) noexcept { return lhs += rhs; }
	friend metre_vec2 operator-(metre_vec2 lhs, metre_vec2 rhs) noexcept { return lhs -= rhs; }
	friend metre_vec2 operator*(metre_vec2 lhs, value_type rhs) noexcept { return lhs *= rhs; }
	friend metre_vec2 operator/(metre_vec2 lhs, value_type rhs) noexcept { return lhs /= rhs; }
	friend metre_vec2 operator*(metre_vec2 lhs, vec_type rhs) noexcept { return lhs *= rhs; }
	friend metre_vec2 operator/(metre_vec2 lhs, vec_type rhs) noexcept { return lhs /= rhs; }

	explicit constexpr operator vec_type() const noexcept {
		vec_type v;
		v.x = static_cast<value_type>(x);
		v.y = static_cast<value_type>(y);
		return v;
	}
};

class metre_vec3 {
public:
	using T = metre;
	using value_type = T::value_type;
	using vec_type = glm::tvec3<value_type>;

	static constexpr auto length = 3;

public:
	T x;
	T y;
	T z;

public:
	metre_vec3() = default;
	constexpr metre_vec3(T x, T y, T z) noexcept : x(x), y(y), z(z) {}
	template <int Exp1, int Exp2, int Exp3>
	constexpr metre_vec3(ste_length_type<Exp1> x, ste_length_type<Exp2> y, ste_length_type<Exp3> z) noexcept : x(x), y(y), z(z) {}
	explicit constexpr metre_vec3(vec_type v) noexcept : x(T(v.x)), y(T(v.y)), z(T(v.z)) {}

	metre_vec3(metre_vec3&&) = default;
	metre_vec3(const metre_vec3&) = default;

	constexpr auto& v() { return *reinterpret_cast<vec_type*>(this); }
	constexpr const auto& v() const { return *reinterpret_cast<const vec_type*>(this); }

	constexpr metre_vec3 &operator=(metre_vec3 &&rhs) noexcept {
		v() = std::move(rhs.v());
		return *this;
	}

	metre_vec3& operator=(const metre_vec3 &rhs) noexcept {
		v() = rhs.v();
		return *this;
	}
	metre_vec3& operator+=(metre_vec3 rhs) noexcept {
		v() += rhs.v();
		return *this;
	}
	metre_vec3& operator-=(metre_vec3 rhs) noexcept {
		v() -= rhs.v();
		return *this;
	}
	metre_vec3& operator*=(value_type rhs) noexcept {
		v() *= rhs;
		return *this;
	}
	metre_vec3& operator/=(value_type rhs) noexcept {
		v() /= rhs;
		return *this;
	}
	metre_vec3& operator*=(vec_type rhs) noexcept {
		v() *= rhs;
		return *this;
	}
	metre_vec3& operator/=(vec_type rhs) noexcept {
		v() /= rhs;
		return *this;
	}
	bool operator==(metre_vec3 rhs) const noexcept { return v() == rhs.v(); }
	bool operator!=(metre_vec3 rhs) const noexcept { return v() != rhs.v(); }
	metre_vec3& operator++() noexcept { ++v(); return *this; }
	metre_vec3& operator--() noexcept { --v(); return *this; }
	metre_vec3 operator++(int) noexcept { return metre_vec3(v()++); }
	metre_vec3 operator--(int) noexcept { return metre_vec3(v()--); }
	metre_vec3 operator+() const noexcept { return metre_vec3(+v()); }
	metre_vec3 operator-() const noexcept { return metre_vec3(-v()); }

	friend metre_vec3 operator+(metre_vec3 lhs, metre_vec3 rhs) noexcept { return lhs += rhs; }
	friend metre_vec3 operator-(metre_vec3 lhs, metre_vec3 rhs) noexcept { return lhs -= rhs; }
	friend metre_vec3 operator*(metre_vec3 lhs, value_type rhs) noexcept { return lhs *= rhs; }
	friend metre_vec3 operator/(metre_vec3 lhs, value_type rhs) noexcept { return lhs /= rhs; }
	friend metre_vec3 operator*(metre_vec3 lhs, vec_type rhs) noexcept { return lhs *= rhs; }
	friend metre_vec3 operator/(metre_vec3 lhs, vec_type rhs) noexcept { return lhs /= rhs; }

	explicit constexpr operator vec_type() const noexcept {
		vec_type v;
		v.x = static_cast<value_type>(x);
		v.y = static_cast<value_type>(y);
		v.z = static_cast<value_type>(z);
		return v;
	}
};

class metre_vec4 {
public:
	using T = metre;
	using value_type = T::value_type;
	using vec_type = glm::tvec4<value_type>;

	static constexpr auto length = 4;

public:
	T x;
	T y;
	T z;
	T w;

public:
	metre_vec4() = default;
	constexpr metre_vec4(T x, T y, T z, T w) noexcept : x(x), y(y), z(z), w(w) {}
	template <int Exp1, int Exp2, int Exp3, int Exp4>
	constexpr metre_vec4(ste_length_type<Exp1> x, ste_length_type<Exp2> y, ste_length_type<Exp3> z, ste_length_type<Exp4> w) noexcept : x(x), y(y), z(z), w(w) {}
	explicit constexpr metre_vec4(vec_type v) noexcept : x(T(v.x)), y(T(v.y)), z(T(v.z)), w(T(v.w)) {}

	metre_vec4(metre_vec4&&) = default;
	metre_vec4(const metre_vec4&) = default;

	constexpr auto& v() { return *reinterpret_cast<vec_type*>(this); }
	constexpr const auto& v() const { return *reinterpret_cast<const vec_type*>(this); }

	constexpr metre_vec4 &operator=(metre_vec4 &&rhs) noexcept {
		v() = std::move(rhs.v());
		return *this;
	}

	metre_vec4& operator=(const metre_vec4 &rhs) noexcept {
		v() = rhs.v();
		return *this;
	}
	metre_vec4& operator+=(metre_vec4 rhs) noexcept {
		v() += rhs.v();
		return *this;
	}
	metre_vec4& operator-=(metre_vec4 rhs) noexcept {
		v() -= rhs.v();
		return *this;
	}
	metre_vec4& operator*=(value_type rhs) noexcept {
		v() *= rhs;
		return *this;
	}
	metre_vec4& operator/=(value_type rhs) noexcept {
		v() /= rhs;
		return *this;
	}
	metre_vec4& operator*=(vec_type rhs) noexcept {
		v() *= rhs;
		return *this;
	}
	metre_vec4& operator/=(vec_type rhs) noexcept {
		v() /= rhs;
		return *this;
	}
	bool operator==(metre_vec4 rhs) const noexcept { return v() == rhs.v(); }
	bool operator!=(metre_vec4 rhs) const noexcept { return v() != rhs.v(); }
	metre_vec4& operator++() noexcept { ++v(); return *this; }
	metre_vec4& operator--() noexcept { --v(); return *this; }
	metre_vec4 operator++(int) noexcept { return metre_vec4(v()++); }
	metre_vec4 operator--(int) noexcept { return metre_vec4(v()--); }
	metre_vec4 operator+() const noexcept { return metre_vec4(+v()); }
	metre_vec4 operator-() const noexcept { return metre_vec4(-v()); }

	friend metre_vec4 operator+(metre_vec4 lhs, metre_vec4 rhs) noexcept { return lhs += rhs; }
	friend metre_vec4 operator-(metre_vec4 lhs, metre_vec4 rhs) noexcept { return lhs -= rhs; }
	friend metre_vec4 operator*(metre_vec4 lhs, value_type rhs) noexcept { return lhs *= rhs; }
	friend metre_vec4 operator/(metre_vec4 lhs, value_type rhs) noexcept { return lhs /= rhs; }
	friend metre_vec4 operator*(metre_vec4 lhs, vec_type rhs) noexcept { return lhs *= rhs; }
	friend metre_vec4 operator/(metre_vec4 lhs, vec_type rhs) noexcept { return lhs /= rhs; }

	explicit constexpr operator vec_type() const noexcept {
		vec_type v;
		v.x = static_cast<value_type>(x);
		v.y = static_cast<value_type>(y);
		v.z = static_cast<value_type>(z);
		v.w = static_cast<value_type>(w);
		return v;
	}
};

}
