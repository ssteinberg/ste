// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>

namespace ste {
namespace Graphics {

class entity {
public:
	virtual ~entity() noexcept {}

	virtual glm::vec3 get_position() const = 0;
};

/**
*	@brief	Transformable entity. Transformation is encoded as a
*	4x3 matrix.
*/
class entity_affine : public entity {
private:
	glm::mat4x3 model_transform_matrix{ 1.f };

public:
	virtual ~entity_affine() noexcept {}

	/**
	*	@brief	Set transform
	*
	* 	@param q	Transform dual-quaternion
	*/
	virtual void set_model_transform(const glm::mat4x3 &m) { model_transform_matrix = m; }

	glm::vec3 get_position() const override {
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;

		glm::decompose(glm::mat4(model_transform_matrix),
					   scale,
					   orientation,
					   translation,
					   skew,
					   perspective);

		return -translation;
	}

	glm::quat get_orientation() const {
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;

		glm::decompose(glm::mat4(model_transform_matrix),
					   scale,
					   orientation,
					   translation,
					   skew,
					   perspective);

		return orientation;
	}

	auto &get_model_transform() const {
		return model_transform_matrix;
	}
};

}
}
