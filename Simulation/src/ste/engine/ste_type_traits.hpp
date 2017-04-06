//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <type_traits>
#include <limits>

#include <half.hpp>

namespace StE {
namespace GL {

// Vector elements count
namespace _detail {
template<typename T> 
struct _type_elements_count_impl {
	static constexpr std::size_t rows = 0; 
	static constexpr std::size_t elements = std::is_arithmetic<T>::value ? 1 : 0;
};
template<>				struct _type_elements_count_impl<half_float::half> {
	static constexpr std::size_t rows = 0; 
	static constexpr std::size_t elements = 1;
};
template<typename vecT> struct _type_elements_count_impl<glm::tvec1<vecT>> {
	static constexpr std::size_t rows = 0; 
	static constexpr std::size_t elements = 1;
};
template<typename vecT> struct _type_elements_count_impl<glm::tvec2<vecT>> {
	static constexpr std::size_t rows = 0; 
	static constexpr std::size_t elements = 2;
};
template<typename vecT> struct _type_elements_count_impl<glm::tvec3<vecT>> {
	static constexpr std::size_t rows = 0; 
	static constexpr std::size_t elements = 3;
};
template<typename vecT> struct _type_elements_count_impl<glm::tvec4<vecT>> {
	static constexpr std::size_t rows = 0; 
	static constexpr std::size_t elements = 4;
};
template<typename matT> struct _type_elements_count_impl<glm::tmat2x2<matT>> {
	static constexpr std::size_t rows = 2;
	static constexpr std::size_t elements = 4;
};
template<typename matT> struct _type_elements_count_impl<glm::tmat2x3<matT>> {
	static constexpr std::size_t rows = 3;
	static constexpr std::size_t elements = 6;
};
template<typename matT> struct _type_elements_count_impl<glm::tmat2x4<matT>> {
	static constexpr std::size_t rows = 4;
	static constexpr std::size_t elements = 8;
};
template<typename matT> struct _type_elements_count_impl<glm::tmat3x2<matT>> {
	static constexpr std::size_t rows = 2;
	static constexpr std::size_t elements = 6;
};
template<typename matT> struct _type_elements_count_impl<glm::tmat3x3<matT>> {
	static constexpr std::size_t rows = 3;
	static constexpr std::size_t elements = 9;
};
template<typename matT> struct _type_elements_count_impl<glm::tmat3x4<matT>> {
	static constexpr std::size_t rows = 4;
	static constexpr std::size_t elements = 12;
};
template<typename matT> struct _type_elements_count_impl<glm::tmat4x2<matT>> {
	static constexpr std::size_t rows = 2;
	static constexpr std::size_t elements = 8;
};
template<typename matT> struct _type_elements_count_impl<glm::tmat4x3<matT>> {
	static constexpr std::size_t rows = 3;
	static constexpr std::size_t elements = 12;
};
template<typename matT> struct _type_elements_count_impl<glm::tmat4x4<matT>> {
	static constexpr std::size_t rows = 4;
	static constexpr std::size_t elements = 16;
};
}

/**
*	@brief	Type element count traits.
*			0 for non arithmetic types. 1 for scalar arithmetics. >1 for vectors and matrices.
*			References and CV qualifiers are ignored.
*/
template<typename T> struct type_elements_count {
	using S = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr std::size_t elements = _detail::_type_elements_count_impl<S>::elements;
};

// Is scalar/vector/matrix
namespace _detail {
template<typename T> struct _is_vector_impl : std::false_type {};
template<typename vecT> struct _is_vector_impl<glm::tvec2<vecT>> : std::true_type {};
template<typename vecT> struct _is_vector_impl<glm::tvec3<vecT>> : std::true_type {};
template<typename vecT> struct _is_vector_impl<glm::tvec4<vecT>> : std::true_type {};
}

/**
*	@brief	Is an arithmetic scalar type trait.
 *			References and CV qualifiers are ignored.
*/
template<typename T> struct is_scalar {
	using S = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr bool value = type_elements_count<S>::elements == 1;
};
/**
*	@brief	Is an arithmetic vector type trait.
*			References and CV qualifiers are ignored.
*/
template<typename T> struct is_vector {
	using S = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr bool value = _detail::_is_vector_impl<S>::value;
};
/**
*	@brief	Is an arithmetic matrix type trait.
*			References and CV qualifiers are ignored.
*/
template<typename T> struct is_matrix {
	using S = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr bool value = type_elements_count<S>::elements > 1 && !_detail::_is_vector_impl<S>::value;
};

/**
*	@brief	Number of matrix rows.
*			0 for non matrix types.
*			References and CV qualifiers are ignored.
*/
template<typename T> struct matrix_rows_count {
	using S = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr std::size_t value = is_matrix<S>::value ? _detail::_type_elements_count_impl<S>::rows : 0;
};
/**
*	@brief	Number of matrix columns.
*			0 for non matrix types.
*			References and CV qualifiers are ignored.
*/
template<typename T> struct matrix_columns_count {
	using S = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr std::size_t value = is_matrix<S>::value ? 
		_detail::_type_elements_count_impl<S>::elements / _detail::_type_elements_count_impl<S>::rows :
		0;
};


// Scalar/vector signness
namespace _detail {
template<typename T>
struct _type_is_signed { static constexpr bool value = std::numeric_limits<T>::is_signed; };
template<> struct _type_is_signed<half_float::half> { static constexpr bool value = true; };
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

/**
*	@brief	Is signed type trait.
*			References and CV qualifiers are ignored.
*/
template<typename T> struct is_signed {
	using TypeT = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr auto value = _detail::_type_is_signed<TypeT>::value;
};

// Scalar/vector is floating point
namespace _detail {
template<typename T>
struct _type_is_floating_point { static constexpr bool value = std::numeric_limits<T>::is_iec559; };
template<> struct _type_is_floating_point<half_float::half> { static constexpr bool value = true; };
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

/**
*	@brief	Is a floating-point type trait.
*			References and CV qualifiers are ignored.
*/
template<typename T> struct is_floating_point {
	using TypeT = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr auto value = _detail::_type_is_floating_point<TypeT>::value;
};

// Vector/matrix underlying type
namespace _detail {
template<typename T> struct _type_remove_extents { using type = std::remove_all_extents_t<T>; };
template<typename vecT> struct _type_remove_extents<glm::tvec1<vecT>> { using type = vecT; };
template<typename vecT> struct _type_remove_extents<glm::tvec2<vecT>> { using type = vecT; };
template<typename vecT> struct _type_remove_extents<glm::tvec3<vecT>> { using type = vecT; };
template<typename vecT> struct _type_remove_extents<glm::tvec4<vecT>> { using type = vecT; };
template<typename matT> struct _type_remove_extents<glm::tmat2x2<matT>> { using type = matT; };
template<typename matT> struct _type_remove_extents<glm::tmat2x3<matT>> { using type = matT; };
template<typename matT> struct _type_remove_extents<glm::tmat2x4<matT>> { using type = matT; };
template<typename matT> struct _type_remove_extents<glm::tmat3x2<matT>> { using type = matT; };
template<typename matT> struct _type_remove_extents<glm::tmat3x3<matT>> { using type = matT; };
template<typename matT> struct _type_remove_extents<glm::tmat3x4<matT>> { using type = matT; };
template<typename matT> struct _type_remove_extents<glm::tmat4x2<matT>> { using type = matT; };
template<typename matT> struct _type_remove_extents<glm::tmat4x3<matT>> { using type = matT; };
template<typename matT> struct _type_remove_extents<glm::tmat4x4<matT>> { using type = matT; };
}

/**
*	@brief	Returns the underlying type of an array, scalar, vector or matrix.
*			References and CV qualifiers are ignored.
*/
template<typename T> struct remove_extents {
	using _TypeT = std::remove_cv_t<std::remove_reference_t<T>>;
	using type = typename _detail::_type_remove_extents<_TypeT>::type;
};


}
}
