// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "dual_quaternion.hpp"

#include <glm/gtx/matrix_decompose.hpp>

namespace StE {
namespace Graphics {

class entity {
public:
	virtual ~entity() noexcept {}

	virtual glm::vec3 get_position() const = 0;
};

/**
*	@brief	Transformable entity. Transformation is encoded as a
*	dual-quaternion.
*/
class entity_dquat : public entity {
private:
	glm::dualquat model_transform;

public:
	virtual ~entity_dquat() noexcept {}

	/**
	*	@brief	Set transform
	*
	* 	@param q	Transform dual-quaternion
	*/
	virtual void set_model_transform(const glm::dualquat &q) { model_transform = q; }

	/**
	*	@brief	Set transform (matrix version)
	*
	*	Rotation and translation will be decomposed from the matrix
	*	and encoded via a dual-quaternion
	*
	* 	@param m	4x4 affine transformation matrix
	*/
	void set_model_matrix(const glm::mat4 &m) {
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;

		glm::decompose(m,
					   scale,
					   orientation,
					   translation,
					   skew,
					   perspective);

		glm::mat3 r = m;
		glm::vec3 t = translation;

		this->set_model_transform(dualquat_translate_rotate(r, t));
	}

	glm::vec3 get_position() const override {
		auto q = (2.f * model_transform.dual * glm::inverse(model_transform.real));
		return { q.x, q.y, q.z };
	}
	const glm::dualquat &get_model_transform() const {
		return model_transform;
	}
};

}
}
