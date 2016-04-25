
#include "stdafx.hpp"
#include "deferred_gbuffer.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void deferred_gbuffer::resize(glm::ivec2 size) {
	if (size.x <= 0 || size.y <= 0)
		return;

	this->size = size;

	gbuffer.commit_range(0, size.x * size.y * 3);

	gbuffer_ll_heads = std::make_unique<Core::Texture2D>(gli::format::FORMAT_R32_UINT_PACK32, glm::ivec2{ size });

	depth_target = std::make_unique<Core::RenderTarget>(gli::format::FORMAT_D32_SFLOAT_PACK32, glm::ivec2{ size });
	fbo.depth_binding_point() = *depth_target;
}

void deferred_gbuffer::bind_gbuffer() const {
	6_storage_idx = gbuffer;
	7_atomic_idx = gbuffer_ll_counter;
	7_image_idx = (*gbuffer_ll_heads)[0];
}
