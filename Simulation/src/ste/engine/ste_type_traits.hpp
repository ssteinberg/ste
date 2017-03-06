//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <type_traits>

#include <half.hpp>

namespace StE {
namespace GL {

// Vector elements count
namespace _detail {
template<typename T> struct _vector_elements_count_impl { static constexpr std::size_t elements = std::is_arithmetic<T>::value ? 1 : 0; };
template<>				struct _vector_elements_count_impl<half_float::half> { static constexpr std::size_t elements = 1; };
template<typename vecT> struct _vector_elements_count_impl<glm::tvec1<vecT>> { static constexpr std::size_t elements = 1; };
template<typename vecT> struct _vector_elements_count_impl<glm::tvec2<vecT>> { static constexpr std::size_t elements = 2; };
template<typename vecT> struct _vector_elements_count_impl<glm::tvec3<vecT>> { static constexpr std::size_t elements = 3; };
template<typename vecT> struct _vector_elements_count_impl<glm::tvec4<vecT>> { static constexpr std::size_t elements = 4; };
template<typename matT> struct _vector_elements_count_impl<glm::tmat2x2<matT>> { static constexpr std::size_t elements = 4; };
template<typename matT> struct _vector_elements_count_impl<glm::tmat2x3<matT>> { static constexpr std::size_t elements = 6; };
template<typename matT> struct _vector_elements_count_impl<glm::tmat2x4<matT>> { static constexpr std::size_t elements = 8; };
template<typename matT> struct _vector_elements_count_impl<glm::tmat3x2<matT>> { static constexpr std::size_t elements = 6; };
template<typename matT> struct _vector_elements_count_impl<glm::tmat3x3<matT>> { static constexpr std::size_t elements = 9; };
template<typename matT> struct _vector_elements_count_impl<glm::tmat3x4<matT>> { static constexpr std::size_t elements = 12; };
template<typename matT> struct _vector_elements_count_impl<glm::tmat4x2<matT>> { static constexpr std::size_t elements = 8; };
template<typename matT> struct _vector_elements_count_impl<glm::tmat4x3<matT>> { static constexpr std::size_t elements = 12; };
template<typename matT> struct _vector_elements_count_impl<glm::tmat4x4<matT>> { static constexpr std::size_t elements = 16; };
}
template<typename T> struct vector_elements_count {
	using CounterT = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr std::size_t elements = _detail::_vector_elements_count_impl<CounterT>::elements;
};

// Scalar/vector signness
namespace _detail {
template<typename T>
struct _type_is_signed {};
template<> struct _type_is_signed<std::int8_t> { static constexpr bool value = true; };
template<> struct _type_is_signed<std::int16_t> { static constexpr bool value = true; };
template<> struct _type_is_signed<std::int32_t> { static constexpr bool value = true; };
template<> struct _type_is_signed<std::int64_t> { static constexpr bool value = true; };
template<> struct _type_is_signed<std::uint8_t> { static constexpr bool value = false; };
template<> struct _type_is_signed<std::uint16_t> { static constexpr bool value = false; };
template<> struct _type_is_signed<std::uint32_t> { static constexpr bool value = false; };
template<> struct _type_is_signed<std::uint64_t> { static constexpr bool value = false; };
template<> struct _type_is_signed<half_float::half> { static constexpr bool value = true; };
template<> struct _type_is_signed<float> { static constexpr bool value = true; };
template<> struct _type_is_signed<double> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::i8vec2> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::i16vec2> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::i32vec2> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::i64vec2> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::u8vec2> { static constexpr bool value = false; };
template<> struct _type_is_signed<glm::u16vec2> { static constexpr bool value = false; };
template<> struct _type_is_signed<glm::u32vec2> { static constexpr bool value = false; };
template<> struct _type_is_signed<glm::u64vec2> { static constexpr bool value = false; };
template<> struct _type_is_signed<glm::tvec2<half_float::half>> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::vec2> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::f64vec2> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::i8vec3> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::i16vec3> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::i32vec3> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::i64vec3> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::u8vec3> { static constexpr bool value = false; };
template<> struct _type_is_signed<glm::u16vec3> { static constexpr bool value = false; };
template<> struct _type_is_signed<glm::u32vec3> { static constexpr bool value = false; };
template<> struct _type_is_signed<glm::u64vec3> { static constexpr bool value = false; };
template<> struct _type_is_signed<glm::tvec3<half_float::half>> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::vec3> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::f64vec3> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::i8vec4> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::i16vec4> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::i32vec4> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::i64vec4> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::u8vec4> { static constexpr bool value = false; };
template<> struct _type_is_signed<glm::u16vec4> { static constexpr bool value = false; };
template<> struct _type_is_signed<glm::u32vec4> { static constexpr bool value = false; };
template<> struct _type_is_signed<glm::u64vec4> { static constexpr bool value = false; };
template<> struct _type_is_signed<glm::tvec4<half_float::half>> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::vec4> { static constexpr bool value = true; };
template<> struct _type_is_signed<glm::f64vec4> { static constexpr bool value = true; };
}
template<typename T> struct type_is_signed {
	using TypeT = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr auto value = _detail::_type_is_signed<TypeT>::format;
};

// Scalar/vector is floating point
namespace _detail {
template<typename T>
struct _type_is_floating_point {};
template<> struct _type_is_floating_point<std::int8_t> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<std::int16_t> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<std::int32_t> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<std::int64_t> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<std::uint8_t> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<std::uint16_t> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<std::uint32_t> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<std::uint64_t> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<half_float::half> { static constexpr bool value = true; };
template<> struct _type_is_floating_point<float> { static constexpr bool value = true; };
template<> struct _type_is_floating_point<double> { static constexpr bool value = true; };
template<> struct _type_is_floating_point<glm::i8vec2> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::i16vec2> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::i32vec2> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::i64vec2> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::u8vec2> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::u16vec2> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::u32vec2> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::u64vec2> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::tvec2<half_float::half>> { static constexpr bool value = true; };
template<> struct _type_is_floating_point<glm::vec2> { static constexpr bool value = true; };
template<> struct _type_is_floating_point<glm::f64vec2> { static constexpr bool value = true; };
template<> struct _type_is_floating_point<glm::i8vec3> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::i16vec3> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::i32vec3> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::i64vec3> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::u8vec3> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::u16vec3> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::u32vec3> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::u64vec3> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::tvec3<half_float::half>> { static constexpr bool value = true; };
template<> struct _type_is_floating_point<glm::vec3> { static constexpr bool value = true; };
template<> struct _type_is_floating_point<glm::f64vec3> { static constexpr bool value = true; };
template<> struct _type_is_floating_point<glm::i8vec4> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::i16vec4> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::i32vec4> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::i64vec4> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::u8vec4> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::u16vec4> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::u32vec4> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::u64vec4> { static constexpr bool value = false; };
template<> struct _type_is_floating_point<glm::tvec4<half_float::half>> { static constexpr bool value = true; };
template<> struct _type_is_floating_point<glm::vec4> { static constexpr bool value = true; };
template<> struct _type_is_floating_point<glm::f64vec4> { static constexpr bool value = true; };
}
template<typename T> struct type_is_floating_point {
	using TypeT = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr auto value = _detail::_type_is_floating_point<TypeT>::format;
};

}
}
