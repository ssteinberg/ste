// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"

#include "resource.hpp"

#include "Texture2D.hpp"
#include "FramebufferObject.hpp"

#include "gl_current_context.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class deferred_fbo {
private:
	std::unique_ptr<LLR::RenderTarget> depth_output;
	std::unique_ptr<LLR::Texture2D> normal_output;
	std::unique_ptr<LLR::Texture2D> tangent_output;
	std::unique_ptr<LLR::Texture2D> position_output;
	std::unique_ptr<LLR::Texture2D> color_output;
	std::unique_ptr<LLR::Texture2D> material_idx_output;
	std::unique_ptr<LLR::Texture2D> z_output;
	LLR::FramebufferObject fbo;

	glm::ivec2 size;

public:
	deferred_fbo(glm::ivec2 size) { resize(size); }

	void resize(glm::ivec2 size);

	void bind_output_textures() const;

	auto get_size() const { return size; }
	const LLR::Texture2D *z_buffer() const { return z_output.get(); }
	const LLR::FramebufferObject *get_fbo() const { return &fbo; }
};

}
}
