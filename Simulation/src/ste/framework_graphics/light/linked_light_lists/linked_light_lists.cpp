
#include "stdafx.hpp"
#include "linked_light_lists.hpp"

using namespace StE::Graphics;

void linked_light_lists::resize(glm::ivec2 size) {
	size = (size + glm::ivec2(7)) / 8;

	if (size.x <= 0 || size.y <= 0)
		return;

	this->size = size;

	lll.commit_range(0, size.x * size.y * 24);
	lll_heads = std::make_unique<Core::Texture2D>(gli::format::FORMAT_R32_UINT_PACK32, glm::ivec2{ size });
}

void linked_light_lists::bind_lll_buffer(bool readonly) const {
	using namespace StE::Core;

	11_storage_idx = lll;
	6_image_idx = (*lll_heads)[0].with_access(readonly ? Core::ImageAccessMode::Read : Core::ImageAccessMode::ReadWrite);

	if (!readonly)
		7_storage_idx = lll_counter;
}
