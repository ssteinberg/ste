// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

namespace ste {

template <typename T>
glm::mat4 reversed_perspective(T fovy, T aspect, T n, T f) {
	const T tan_half_fovy = glm::tan(fovy / static_cast<T>(2));

	glm::tmat4x4<T> m(static_cast<T>(0));
	m[0][0] = static_cast<T>(1) / (aspect * tan_half_fovy);
	m[1][1] = static_cast<T>(1) / (tan_half_fovy);
	m[2][2] = (n) / (f - n);
	m[2][3] = -static_cast<T>(1);
	m[3][2] = n * f / (f - n);
	return m;
}

template <typename T>
glm::mat4 reversed_infinite_perspective(T fovy, T aspect, T n) {
	const T tan_half_fovy = glm::tan(fovy / static_cast<T>(2));

	glm::tmat4x4<T> m(static_cast<T>(0));
	m[0][0] = static_cast<T>(1) / (aspect * tan_half_fovy);
	m[1][1] = static_cast<T>(1) / (tan_half_fovy);
	m[2][3] = static_cast<T>(-1);
	m[3][2] = n;
	return m;
}

}
