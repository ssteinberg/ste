// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "Texture2D.h"
#include "FramebufferObject.h"

#include <memory>
#include <array>

namespace StE {
namespace Graphics {

class deferred_fbo {
private:
	std::unique_ptr<LLR::RenderTarget> depth_output;
	std::unique_ptr<LLR::Texture2D> normal_output;
	std::unique_ptr<LLR::Texture2D> tangent_output;
	std::unique_ptr<LLR::Texture2D> position_output;
	std::unique_ptr<LLR::Texture2D> uv_output;
	std::unique_ptr<LLR::Texture2D> duv_output;
	std::unique_ptr<LLR::Texture2D> material_idx_output;
	std::unique_ptr<LLR::Texture2D> z_output;
	LLR::FramebufferObject fbo;

	std::array<unsigned, 6> textures;

	glm::ivec2 size;

public:
	deferred_fbo(glm::ivec2 size) { resize(size); }

	void resize(glm::ivec2 size) {
		if (size.x <= 0 || size.y <= 0)
			return;

		this->size = size;

		depth_output = std::make_unique<LLR::RenderTarget>(gli::format::FORMAT_D24_UNORM, glm::tvec2<std::size_t>{ size });
		normal_output = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RGB32_SFLOAT, glm::tvec2<std::size_t>{ size }, 1);
		tangent_output = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RGB32_SFLOAT, glm::tvec2<std::size_t>{ size }, 1);
		position_output = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RGB32_SFLOAT, glm::tvec2<std::size_t>{ size }, 1);
		uv_output = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RG32_SFLOAT, glm::tvec2<std::size_t>{ size }, 1);
		duv_output = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_RGBA32_SFLOAT, glm::tvec2<std::size_t>{ size }, 1);
		material_idx_output = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_R16_SINT, glm::tvec2<std::size_t>{ size }, 1);
		z_output = std::make_unique<LLR::Texture2D>(gli::format::FORMAT_R32_SFLOAT, glm::tvec2<std::size_t>{ size }, 1);

		textures[0] = normal_output->get_resource_id();
		textures[1] = position_output->get_resource_id();
		textures[2] = uv_output->get_resource_id();
		textures[3] = duv_output->get_resource_id();
		textures[4] = tangent_output->get_resource_id();
		textures[5] = material_idx_output->get_resource_id();

		fbo.depth_binding_point() = *depth_output;
		fbo[0] = (*position_output)[0];
		fbo[1] = (*uv_output)[0];
		fbo[2] = (*duv_output)[0];
		fbo[3] = (*normal_output)[0];
		fbo[4] = (*z_output)[0];
		fbo[5] = (*tangent_output)[0];
		fbo[6] = (*material_idx_output)[0];
	}

	void bind_output_textures() const {
		glBindTextures(0, 6, &textures[0]);
	}

	auto get_size() const { return size; }
	const LLR::Texture2D *z_buffer() const { return z_output.get(); }
	const LLR::FramebufferObject *get_fbo() const { return &fbo; }
};

}
}
