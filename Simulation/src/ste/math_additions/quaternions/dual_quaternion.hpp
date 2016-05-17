// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include <glm/gtx/dual_quaternion.hpp>

namespace StE {

/**
*	@brief	dual quaternion from rotation, translation
*
*	Applies rotation and translation in that order.
*
* 	@param r	3x3 orthogonal rotation matrix
* 	@param t	translation vector
*/
template <typename T, glm::precision P>
glm::tdualquat<T, P> dualquat_rotate_translate(const glm::tmat3x3<T, P> &r, const glm::tvec3<T, P> &t) {
	auto qr = glm::tquat<T, P>(r);
	auto qt = glm::tquat<T, P>(.0f, t);
	return glm::tdualquat<T, P>(qr, .5f * qt * qr);
}

/**
*	@brief	dual quaternion from translation, rotation
*
*	Applies translation and rotation in that order.
*
* 	@param r	3x3 orthogonal rotation matrix
* 	@param t	translation vector
*/
template <typename T, glm::precision P>
glm::tdualquat<T, P> dualquat_translate_rotate(const glm::tmat3x3<T, P> &r, const glm::tvec3<T, P> &t) {
	auto qr = glm::tquat<T, P>(r);
	auto qt = glm::tquat<T, P>(.0f, t);
	return glm::tdualquat<T, P>(qr, .5f * qr * qt);
}

}
