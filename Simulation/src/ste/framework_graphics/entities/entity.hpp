// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include <glm/gtx/matrix_decompose.hpp>

namespace StE {
namespace Graphics {
	
class entity {
public:
	virtual ~entity() noexcept {}
	
	virtual glm::vec3 get_position() const = 0;
};

class entity_affine : public entity {
protected:
	glm::mat4 model_mat{ 1.f };
	
public:
	virtual ~entity_affine() noexcept {}
	
	virtual void set_model_matrix(const glm::mat4 &m) { model_mat = m; }
	const glm::mat4 &get_model_transform() const { return model_mat; }
	
	glm::vec3 get_position() const override {
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;

		glm::decompose(model_mat,
					   scale,
					   orientation,
					   translation,
					   skew,
					   perspective);

		return translation;
	}
};
	
}
}
