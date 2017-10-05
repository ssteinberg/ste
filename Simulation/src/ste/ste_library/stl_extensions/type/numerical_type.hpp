//  StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <type_traits>

namespace ste {

/*
 *	@brief	Strong-typed wrapper around a numerical type T
 */
template<class T>
class numerical_type {
public:
	using value_type = T;

protected:
	value_type val;

public:
	constexpr numerical_type() = default;
	explicit constexpr numerical_type(value_type v) noexcept : val(v) {}

	numerical_type(numerical_type&&) = default;
	numerical_type(const numerical_type&) = default;
	
	constexpr numerical_type &operator=(numerical_type &&rhs) noexcept {
		val = std::move(rhs.val);
		return *this;
	}

	constexpr numerical_type& operator=(const numerical_type &rhs) noexcept {
		val = rhs.val;
		return *this;
	}
	constexpr numerical_type& operator+=(numerical_type rhs) noexcept {
		val += rhs.val;
		return *this;
	}
	constexpr numerical_type& operator-=(numerical_type rhs) noexcept {
		val -= rhs.val;
		return *this;
	}
	constexpr numerical_type& operator*=(value_type rhs) noexcept {
		val *= rhs;
		return *this;
	}
	constexpr numerical_type& operator/=(value_type rhs) noexcept {
		val /= rhs;
		return *this;
	}
	constexpr numerical_type& operator++() noexcept { ++val; return *this; }
	constexpr numerical_type& operator--() noexcept { --val; return *this; }
	constexpr numerical_type operator++(int) noexcept { return numerical_type(val++); }
	constexpr numerical_type operator--(int) noexcept { return numerical_type(val--); }

	template <typename S = value_type, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr numerical_type& operator%=(value_type rhs) noexcept {
		val %= rhs;
		return *this;
	}
	template <typename S = value_type, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr numerical_type& operator&=(value_type rhs) noexcept {
		val &= rhs;
		return *this;
	}
	template <typename S = value_type, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr numerical_type& operator|=(value_type rhs) noexcept {
		val |= rhs;
		return *this;
	}
	template <typename S = value_type, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr numerical_type& operator^=(value_type rhs) noexcept {
		val ^= rhs;
		return *this;
	}
	template <typename S = value_type, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr numerical_type& operator<<=(value_type rhs) noexcept {
		val <<= rhs;
		return *this;
	}
	template <typename S = value_type, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr numerical_type& operator>>=(value_type rhs) noexcept {
		val >>= rhs;
		return *this;
	}

	constexpr bool operator==(numerical_type rhs) const noexcept { return val == rhs.val; }
	constexpr bool operator!=(numerical_type rhs) const noexcept { return val != rhs.val; }

	constexpr numerical_type operator+() const noexcept { return numerical_type(+val); }
	constexpr numerical_type operator-() const noexcept { return numerical_type(-val); }

	constexpr auto operator!() const noexcept { return !val; }
	template <typename S = value_type, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr numerical_type operator~() const noexcept {
		return numerical_type(~val);
	}

	constexpr friend numerical_type operator+(numerical_type lhs, numerical_type rhs) noexcept { return lhs += rhs; }
	constexpr friend numerical_type operator-(numerical_type lhs, numerical_type rhs) noexcept { return lhs -= rhs; }
	constexpr friend numerical_type operator*(numerical_type lhs, value_type rhs) noexcept { return lhs *= rhs; }
	constexpr friend numerical_type operator/(numerical_type lhs, value_type rhs) noexcept { return lhs /= rhs; }

	template <typename S = value_type, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr friend numerical_type operator%(numerical_type lhs, value_type rhs) noexcept { return lhs %= rhs; }
	template <typename S = value_type, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr friend numerical_type operator&(numerical_type lhs, value_type rhs) noexcept { return lhs &= rhs; }
	template <typename S = value_type, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr friend numerical_type operator|(numerical_type lhs, value_type rhs) noexcept { return lhs |= rhs; }
	template <typename S = value_type, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr friend numerical_type operator^(numerical_type lhs, value_type rhs) noexcept { return lhs ^= rhs; }
	template <typename S = value_type, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr friend numerical_type operator<<(numerical_type lhs, value_type rhs) noexcept { return lhs <<= rhs; }
	template <typename S = value_type, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr friend numerical_type operator>>(numerical_type lhs, value_type rhs) noexcept { return lhs >>= rhs; }

	explicit constexpr operator value_type() const noexcept { return val; }
};

}
