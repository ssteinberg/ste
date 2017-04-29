//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <type_traits>
#include <safe_numerical_limits.hpp>

#include <half.hpp>

namespace ste {
namespace gl {

// Vector elements count
namespace _detail {
template<typename T> 
struct _type_elements_count_impl {
	static constexpr std::size_t rows = std::is_arithmetic_v<T> ? 1 : 0;
	static constexpr std::size_t elements = std::is_arithmetic_v<T> ? 1 : 0;
};
template<>				struct _type_elements_count_impl<half_float::half> {
	static constexpr std::size_t rows = 1; 
	static constexpr std::size_t elements = 1;
};
template<typename vecT> struct _type_elements_count_impl<glm::tvec1<vecT>> {
	static constexpr std::size_t rows = 1; 
	static constexpr std::size_t elements = 1;
};
template<typename vecT> struct _type_elements_count_impl<glm::tvec2<vecT>> {
	static constexpr std::size_t rows = 2; 
	static constexpr std::size_t elements = 2;
};
template<typename vecT> struct _type_elements_count_impl<glm::tvec3<vecT>> {
	static constexpr std::size_t rows = 3; 
	static constexpr std::size_t elements = 3;
};
template<typename vecT> struct _type_elements_count_impl<glm::tvec4<vecT>> {
	static constexpr std::size_t rows = 4; 
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
	static constexpr std::size_t value = _detail::_type_elements_count_impl<S>::elements;
};
template<typename T>
static constexpr auto type_elements_count_v = type_elements_count<T>::value;

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
	static constexpr bool value = type_elements_count<S>::value == 1;
};
template<typename T>
static constexpr auto is_scalar_v = is_scalar<T>::value;
/**
*	@brief	Is an arithmetic vector type trait.
*			References and CV qualifiers are ignored.
*/
template<typename T> struct is_vector {
	using S = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr bool value = _detail::_is_vector_impl<S>::value;
};
template<typename T>
static constexpr auto is_vector_v = is_vector<T>::value;
/**
*	@brief	Is an arithmetic matrix type trait.
*			References and CV qualifiers are ignored.
*/
template<typename T> struct is_matrix {
	using S = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr bool value = type_elements_count<S>::value > 1 && !_detail::_is_vector_impl<S>::value;
};
template<typename T>
static constexpr auto is_matrix_v = is_matrix<T>::value;
/**
*	@brief	Is an arithmetic type trait.
*			References and CV qualifiers are ignored.
*/
template<typename T> struct is_arithmetic {
	using S = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr bool value = type_elements_count<S>::value > 0;
};
template<typename T>
static constexpr auto is_arithmetic_v = is_arithmetic<T>::value;

/**
*	@brief	Number of vector/matrix rows.
*			1 for non vector/matrix types.
*			References and CV qualifiers are ignored.
*/
template<typename T> struct matrix_rows_count {
	using S = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr std::size_t value = _detail::_type_elements_count_impl<S>::rows;
};
template<typename T>
static constexpr auto matrix_rows_count_v = matrix_rows_count<T>::value;
/**
*	@brief	Number of matrix columns.
*			1 for non matrix types.
*			References and CV qualifiers are ignored.
*/
template<typename T> struct matrix_columns_count {
	using S = std::remove_cv_t<std::remove_reference_t<T>>;
	static constexpr std::size_t value = is_matrix<S>::value ? 
		_detail::_type_elements_count_impl<S>::elements / _detail::_type_elements_count_impl<S>::rows :
		1;
};
template<typename T>
static constexpr auto matrix_columns_count_v = matrix_columns_count<T>::value;


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
template<typename T>
using remove_extents_t = typename remove_extents<T>::type;


/**
*	@brief	Is signed type trait.
*			References and CV qualifiers are ignored.
*/
template<typename T> struct is_signed {
	using UnderlyingT = remove_extents_t<T>;
	static constexpr auto value = safe_numerical_limits_t<UnderlyingT>::is_signed;
};
template<typename T>
static constexpr auto is_signed_v = is_signed<T>::value;

/**
*	@brief	Is a floating-point type trait.
*			References and CV qualifiers are ignored.
*/
template<typename T> struct is_floating_point {
	using UnderlyingT = remove_extents_t<T>;
	static constexpr auto value = safe_numerical_limits_t<UnderlyingT>::is_iec559;
};
template<typename T>
static constexpr auto is_floating_point_v = is_floating_point<T>::value;


}
}
