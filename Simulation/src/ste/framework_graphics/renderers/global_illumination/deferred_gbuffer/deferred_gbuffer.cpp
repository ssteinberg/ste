
#include "stdafx.hpp"
#include "deferred_gbuffer.hpp"

using namespace StE::Graphics;
using namespace StE::Core;

void deferred_gbuffer::resize(glm::ivec2 size) {
	if (size.x <= 0 || size.y <= 0)
		return;

	this->size = size;

	gbuffer.commit_range(0, size.x * size.y);

	depth_target = std::make_unique<Core::Texture2D>(gli::format::FORMAT_D32_SFLOAT_PACK32, glm::ivec2{ size });
	downsampled_depth_target = std::make_unique<Core::Texture2D>(gli::format::FORMAT_R32_SFLOAT_PACK32, glm::ivec2{ size } / 2, depth_buffer_levels - 1);
	fbo.depth_binding_point() = *depth_target;
	
	backface_depth_target = std::make_unique<Core::Texture2D>(gli::format::FORMAT_D32_SFLOAT_PACK32, glm::ivec2{ size });
	backface_fbo.depth_binding_point() = *backface_depth_target;

	depth_target_modified_signal.emit();
}

void deferred_gbuffer::bind_gbuffer() const {
	using namespace Core;

	6_storage_idx = gbuffer;
}
