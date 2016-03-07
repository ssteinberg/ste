// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "renderable.h"
#include "mesh.h"
#include "gl_current_context.h"
#include "gl_type_traits.h"

#include <memory>

namespace StE {
namespace Graphics {

template <mesh_subdivion_mode mode>
class MeshRenderable : public renderable {
private:
	std::shared_ptr<mesh<mode>> meshptr;

public:
	MeshRenderable(const std::shared_ptr<LLR::GLSLProgram> &p, const std::shared_ptr<mesh<mode>> &m) : renderable(p), meshptr(m) {
		request_state({ GL_DEPTH_TEST, true });
	}
	virtual ~MeshRenderable() noexcept {}

	virtual void prepare() const override {
		renderable::prepare();
		meshptr->vao()->bind();
		meshptr->ebo()->bind();
	}

	virtual void render() const override {
		glDrawElements(static_cast<GLenum>(mode), meshptr->ebo()->size(), LLR::gl_type_name_enum<typename mesh<mode>::ebo_type::T>::gl_enum, nullptr);
	}
};

}
}
