
#include "stdafx.hpp"
#include "linked_light_lists.hpp"

#include "light_storage.hpp"

using namespace StE::Graphics;

constexpr int linked_light_lists::lll_image_res_multiplier;

void linked_light_lists::resize(glm::ivec2 size) {
	size = (size + glm::ivec2(lll_image_res_multiplier - 1)) / lll_image_res_multiplier;

	if (size.x <= 0 || size.y <= 0)
		return;

	this->size = size;

	lll.commit_range(0, size.x * size.y * (max_active_lights_per_frame / 4 * 3));
	lll_heads = std::make_unique<Core::Texture2D>(gli::format::FORMAT_R32_UINT_PACK32, glm::ivec2{ size });
}

void linked_light_lists::bind_lll_buffer(bool readonly) const {
	using namespace StE::Core;

	11_storage_idx = lll;
	6_image_idx = (*lll_heads)[0].with_access(readonly ? Core::ImageAccessMode::Read : Core::ImageAccessMode::ReadWrite);

	if (!readonly)
		7_storage_idx = lll_counter;
}
