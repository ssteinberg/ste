
#include "stdafx.hpp"
#include "deferred_gbuffer.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void deferred_gbuffer::resize(glm::ivec2 size) {
	if (size.x <= 0 || size.y <= 0)
		return;

	this->size = size;

	gbuffer.commit_range(0, size.x * size.y * 6);

	gbuffer_ll_heads = std::make_unique<Core::Texture2D>(gli::format::FORMAT_R32_UINT_PACK32, glm::ivec2{ size });

	depth_target = std::make_unique<Core::Texture2D>(gli::format::FORMAT_D32_SFLOAT_PACK32, glm::ivec2{ size }, depth_buffer_levels);
	fbo.depth_binding_point() = *depth_target;
}

void deferred_gbuffer::bind_gbuffer(bool readonly) const {
	6_storage_idx = gbuffer;
	7_image_idx = (*gbuffer_ll_heads)[0].with_access(readonly ? Core::ImageAccessMode::Read : Core::ImageAccessMode::ReadWrite);

	if (!readonly)
		7_atomic_idx = gbuffer_ll_counter;
}
