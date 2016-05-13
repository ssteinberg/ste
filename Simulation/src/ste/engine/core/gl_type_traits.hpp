// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include <type_traits>
#include <cstdint>
#include <glm/glm.hpp>

namespace StE {
namespace Core {

template<typename T> struct _gl_elements_count_impl { static constexpr std::size_t elements = std::is_arithmetic<T>::value ? 1 : 0; };
template<typename vecT> struct _gl_elements_count_impl<glm::tvec1<vecT>> { static constexpr std::size_t elements = 1; };
template<typename vecT> struct _gl_elements_count_impl<glm::tvec2<vecT>> { static constexpr std::size_t elements = 2; };
template<typename vecT> struct _gl_elements_count_impl<glm::tvec3<vecT>> { static constexpr std::size_t elements = 3; };
template<typename vecT> struct _gl_elements_count_impl<glm::tvec4<vecT>> { static constexpr std::size_t elements = 4; };
template<typename matT> struct _gl_elements_count_impl<glm::tmat2x2<matT>> { static constexpr std::size_t elements = 4; };
template<typename matT> struct _gl_elements_count_impl<glm::tmat2x3<matT>> { static constexpr std::size_t elements = 6; };
template<typename matT> struct _gl_elements_count_impl<glm::tmat2x4<matT>> { static constexpr std::size_t elements = 8; };
template<typename matT> struct _gl_elements_count_impl<glm::tmat3x2<matT>> { static constexpr std::size_t elements = 6; };
template<typename matT> struct _gl_elements_count_impl<glm::tmat3x3<matT>> { static constexpr std::size_t elements = 9; };
template<typename matT> struct _gl_elements_count_impl<glm::tmat3x4<matT>> { static constexpr std::size_t elements = 12; };
template<typename matT> struct _gl_elements_count_impl<glm::tmat4x2<matT>> { static constexpr std::size_t elements = 8; };
template<typename matT> struct _gl_elements_count_impl<glm::tmat4x3<matT>> { static constexpr std::size_t elements = 12; };
template<typename matT> struct _gl_elements_count_impl<glm::tmat4x4<matT>> { static constexpr std::size_t elements = 16; };
template<typename T> struct gl_elements_count{
	using CounterT = std::remove_const_t<std::remove_reference_t<T>>;
	static constexpr std::size_t elements = _gl_elements_count_impl<CounterT>::elements;
};

template<typename T> struct _gl_type_name_enum_impl { static constexpr GLenum gl_enum = 0; };
template<> struct _gl_type_name_enum_impl<bool> { static constexpr GLenum gl_enum = GL_BOOL; };
template<> struct _gl_type_name_enum_impl<std::int8_t> { static constexpr GLenum gl_enum = GL_BYTE; };
template<> struct _gl_type_name_enum_impl<std::uint8_t> { static constexpr GLenum gl_enum = GL_UNSIGNED_BYTE; };
template<> struct _gl_type_name_enum_impl<std::int16_t> { static constexpr GLenum gl_enum = GL_SHORT; };
template<> struct _gl_type_name_enum_impl<std::uint16_t> { static constexpr GLenum gl_enum = GL_UNSIGNED_SHORT; };
template<> struct _gl_type_name_enum_impl<std::int32_t> { static constexpr GLenum gl_enum = GL_INT; };
template<> struct _gl_type_name_enum_impl<std::uint32_t> { static constexpr GLenum gl_enum = GL_UNSIGNED_INT; };
template<> struct _gl_type_name_enum_impl<float> { static constexpr GLenum gl_enum = GL_FLOAT; };
template<> struct _gl_type_name_enum_impl<double> { static constexpr GLenum gl_enum = GL_DOUBLE; };
template<typename vecT> struct _gl_type_name_enum_impl<glm::tvec1<vecT>> : public _gl_type_name_enum_impl<vecT>{};
template<typename vecT> struct _gl_type_name_enum_impl<glm::tvec2<vecT>> : public _gl_type_name_enum_impl<vecT>{};
template<typename vecT> struct _gl_type_name_enum_impl<glm::tvec3<vecT>> : public _gl_type_name_enum_impl<vecT>{};
template<typename vecT> struct _gl_type_name_enum_impl<glm::tvec4<vecT>> : public _gl_type_name_enum_impl<vecT>{};
template<typename matT> struct _gl_type_name_enum_impl<glm::tmat2x2<matT>> : public _gl_type_name_enum_impl<matT>{};
template<typename matT> struct _gl_type_name_enum_impl<glm::tmat2x3<matT>> : public _gl_type_name_enum_impl<matT>{};
template<typename matT> struct _gl_type_name_enum_impl<glm::tmat2x4<matT>> : public _gl_type_name_enum_impl<matT>{};
template<typename matT> struct _gl_type_name_enum_impl<glm::tmat3x2<matT>> : public _gl_type_name_enum_impl<matT>{};
template<typename matT> struct _gl_type_name_enum_impl<glm::tmat3x3<matT>> : public _gl_type_name_enum_impl<matT>{};
template<typename matT> struct _gl_type_name_enum_impl<glm::tmat3x4<matT>> : public _gl_type_name_enum_impl<matT>{};
template<typename matT> struct _gl_type_name_enum_impl<glm::tmat4x2<matT>> : public _gl_type_name_enum_impl<matT>{};
template<typename matT> struct _gl_type_name_enum_impl<glm::tmat4x3<matT>> : public _gl_type_name_enum_impl<matT>{};
template<typename matT> struct _gl_type_name_enum_impl<glm::tmat4x4<matT>> : public _gl_type_name_enum_impl<matT>{};
template<typename T> struct gl_type_name_enum {
	using TypeT = std::remove_const_t<std::remove_reference_t<T>>;
	static constexpr GLenum gl_enum = _gl_type_name_enum_impl<TypeT>::gl_enum;
};

template<typename T> struct gl_type_is_floating_point {
	using TypeT = std::remove_const_t<std::remove_reference_t<T>>;
	static constexpr bool value = gl_type_name_enum<TypeT>::gl_enum == GL_FLOAT || gl_type_name_enum<TypeT>::gl_enum == GL_DOUBLE;
};

template<typename T> struct gl_type_is_signed {
	using TypeT = std::remove_const_t<std::remove_reference_t<T>>;
	static constexpr bool value = !(gl_type_name_enum<TypeT>::gl_enum == GL_UNSIGNED_BYTE || gl_type_name_enum<TypeT>::gl_enum == GL_UNSIGNED_SHORT || gl_type_name_enum<TypeT>::gl_enum == GL_UNSIGNED_INT);
};

}
}
