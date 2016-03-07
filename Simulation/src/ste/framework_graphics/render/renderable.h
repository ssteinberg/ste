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

	void operator()() const {
		this->prepare();
		this->render();
		this->finalize();
	}

	void operator()(std::shared_ptr<LLR::GLSLProgram> override_program, const LLR::GenericFramebufferObject *override_fbo = nullptr) {
		const LLR::GenericFramebufferObject *renderable_fbo = fbo;
		std::shared_ptr<LLR::GLSLProgram> renderable_program = program;

		if (override_program != nullptr)
			program = override_program;
		if (override_fbo != nullptr)
			fbo = override_fbo;

		(*this)();

		if (override_program != nullptr)
			program = renderable_program;
		if (override_fbo != nullptr)
			fbo = renderable_fbo;
	}

	const auto get_program() const { return program.get(); }
	const auto get_output_fbo() const { return fbo; }
	const std::unordered_map<GLenum, bool>& get_requested_states() const { return requested_states; }
};

}
}
