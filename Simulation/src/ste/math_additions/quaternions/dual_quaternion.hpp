//  StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <glm/gtx/dual_quaternion.hpp>

namespace ste {

/**
*	@brief	dual quaternion from rotation, translation
*
*	Applies rotation and translation in that order.
*
* 	@param r	3x3 orthogonal rotation matrix
* 	@param t	translation vector
*/
template <typename T, glm::qualifier Q>
glm::tdualquat<T, Q> dualquat_rotate_translate(const glm::tmat3x3<T, Q> &r, const glm::tvec3<T, Q> &t) {
	auto qr = glm::tquat<T, Q>(r);
	auto qt = glm::tquat<T, Q>(.0f, t);
	return glm::tdualquat<T, Q>(qr, .5f * qt * qr);
}

/**
*	@brief	dual quaternion from translation, rotation
*
*	Applies translation and rotation in that order.
*
* 	@param r	3x3 orthogonal rotation matrix
* 	@param t	translation vector
*/
template <typename T, glm::qualifier Q>
glm::tdualquat<T, Q> dualquat_translate_rotate(const glm::tmat3x3<T, Q> &r, const glm::tvec3<T, Q> &t) {
	auto qr = glm::tquat<T, Q>(r);
	auto qt = glm::tquat<T, Q>(.0f, t);
	return glm::tdualquat<T, Q>(qr, .5f * qr * qt);
}

}
