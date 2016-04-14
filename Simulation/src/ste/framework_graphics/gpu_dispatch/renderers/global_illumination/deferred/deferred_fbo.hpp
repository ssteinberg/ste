// StE
// © Shlomi Steinberg, 2015-2016

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
	std::unique_ptr<Core::RenderTarget> depth_output;
	std::unique_ptr<Core::Texture2D> normal_output;
	std::unique_ptr<Core::Texture2D> tangent_output;
	std::unique_ptr<Core::Texture2D> position_output;
	std::unique_ptr<Core::Texture2D> color_output;
	std::unique_ptr<Core::Texture2D> material_idx_output;
	std::unique_ptr<Core::Texture2D> z_output;
	std::unique_ptr<Core::Texture2D> wposition_output;
	Core::FramebufferObject fbo;

	glm::ivec2 size;

public:
	deferred_fbo(glm::ivec2 size) { resize(size); }

	void resize(glm::ivec2 size);

	void bind_output_textures() const;

	auto get_size() const { return size; }
	const Core::Texture2D *z_buffer() const { return z_output.get(); }
	const Core::FramebufferObject *get_fbo() const { return &fbo; }
};

}
}
