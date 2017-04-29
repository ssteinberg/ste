//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <half.hpp>

#include <type_traits>

namespace ste {
namespace gl {

namespace _detail {

template <typename T, bool is_float>
struct trect_make_unsigned {
	using type = std::make_unsigned_t<T>;
};
template <typename T>
struct trect_make_unsigned<T, true> {
	using type = T;
};

}

template <typename T>
struct trect {
	static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");
	static_assert(!std::is_same_v<bool, T>, "T can not be bool");
	static_assert(std::numeric_limits<T>::is_signed, "T must be a signed arithmetic type");

	// Extent should be unsigned for integer types
	static constexpr bool _is_floating_type = std::numeric_limits<T>::is_iec559;
	using _size_element_t = typename _detail::trect_make_unsigned<T, _is_floating_type>::type;

	using origin_t = glm::tvec2<T>;
	using size_t = glm::tvec2<_size_element_t>;

	origin_t origin;
	size_t size;

	trect(const size_t &size)
		: origin(static_cast<T>(0)), size(size) {}
	trect(const origin_t &origin,
		  const size_t &size)
		: origin(origin), size(size) {}

	trect(trect&&) = default;
	trect(const trect&) = default;
	trect &operator=(trect&&) = default;
	trect &operator=(const trect&) = default;
};

using f16rect = trect<half_float::half>;
using f32rect = trect<float>;
using f64rect = trect<double>;
using i8rect = trect<std::int8_t>;
using i16rect = trect<std::int16_t>;
using i32rect = trect<std::int32_t>;
using i64rect = trect<std::int64_t>;

using rect = f32rect;

}
}
