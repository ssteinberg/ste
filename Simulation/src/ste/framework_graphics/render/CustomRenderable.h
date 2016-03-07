// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "renderable.h"

#include <functional>

namespace StE {
namespace Graphics {

class CustomRenderable : public renderable {
private:
	std::function<void(void)> prepare_lambda, render_lambda, finalize_lambda;

public:
	CustomRenderable(const std::shared_ptr<LLR::GLSLProgram> &p, std::function<void(void)> &&prepare_lambda, std::function<void(void)> &&render_lambda, std::function<void(void)> &&finalize_lambda) : renderable(p), prepare_lambda(std::move(prepare_lambda)), render_lambda(std::move(render_lambda)), finalize_lambda(std::move(finalize_lambda)) {}
	CustomRenderable(const std::shared_ptr<LLR::GLSLProgram> &p, std::function<void(void)> &&render_lambda) : renderable(p), render_lambda(std::move(render_lambda)) {}
	CustomRenderable(std::function<void(void)> &&render_lambda) : renderable(), prepare_lambda([]() {}), render_lambda(std::move(render_lambda)), finalize_lambda([]() {}) {}

	void prepare() const override final {
		renderable::prepare();
		prepare_lambda();
	}

	void render() const override final {
		render_lambda();
	}

	void finalize() const override final {
		finalize_lambda();
	}
};

}
}
