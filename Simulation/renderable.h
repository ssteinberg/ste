// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "GLSLProgram.h"

#include "FramebufferObject.h"

#include <unordered_map>
#include <vector>

#include <memory>
#include <functional>

namespace StE {
namespace Graphics {

class renderable {
protected:
	const LLR::GenericFramebufferObject *fbo{ nullptr };
	std::shared_ptr<LLR::GLSLProgram> program;
	std::unordered_map<GLenum, bool> requested_states;

public:
	renderable() = default;
	renderable(const std::shared_ptr<LLR::GLSLProgram> &p) : program(p) {}
	renderable(const std::shared_ptr<LLR::GLSLProgram> &p, const LLR::GenericFramebufferObject *f) : fbo(f), program(p) {}
	virtual ~renderable() noexcept {}

	virtual void prepare() const {
		assert(fbo != nullptr);

		if (program)
			program->bind();
		fbo->bind();
	}
	virtual void render() const = 0;
	virtual void finalize() const {}

	void set_output_fbo(const LLR::GenericFramebufferObject *p) { fbo = p; }

	void request_state(const std::pair<GLenum, bool> &state) {
		requested_states.insert(state);
	}
	void request_states(const std::vector<std::pair<GLenum, bool>> &states) {
		for (auto &s : states)
			requested_states.insert(s);
	}

	const auto get_program() const { return program.get(); }
	const auto get_fbo() const { return fbo; }
	const std::unordered_map<GLenum, bool>& get_requested_states() const { return requested_states; }
};

}
}
