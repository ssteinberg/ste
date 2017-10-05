//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_type_traits.hpp>

#include <limits>

namespace ste {

namespace _detail {

template <typename N, typename F>
class normalized_type {
private:
	N data;

public:
	normalized_type() = default;
	normalized_type(N t) : data(std::move(t)) {}

	operator N() { return data; }
	operator N() const { return data; }
};

}

using i8_norm = _detail::normalized_type<glm::i8, float>;
using u8_norm = _detail::normalized_type<glm::u8, float>;
using i16_norm = _detail::normalized_type<glm::i16, float>;
using u16_norm = _detail::normalized_type<glm::u16, float>;
using i8vec2_norm = _detail::normalized_type<glm::i8vec2, float>;
using u8vec2_norm = _detail::normalized_type<glm::u8vec2, float>;
using i16vec2_norm = _detail::normalized_type<glm::i16vec2, float>;
using u16vec2_norm = _detail::normalized_type<glm::u16vec2, float>;
using i8vec3_norm = _detail::normalized_type<glm::i8vec3, float>;
using u8vec3_norm = _detail::normalized_type<glm::u8vec3, float>;
using i16vec3_norm = _detail::normalized_type<glm::i16vec3, float>;
using u16vec3_norm = _detail::normalized_type<glm::u16vec3, float>;
using i8vec4_norm = _detail::normalized_type<glm::i8vec4, float>;
using u8vec4_norm = _detail::normalized_type<glm::u8vec4, float>;
using i16vec4_norm = _detail::normalized_type<glm::i16vec4, float>;
using u16vec4_norm = _detail::normalized_type<glm::u16vec4, float>;

}
