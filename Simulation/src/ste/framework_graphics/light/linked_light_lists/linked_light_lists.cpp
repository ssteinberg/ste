
#include "stdafx.hpp"
#include "linked_light_lists.hpp"

#include "light_storage.hpp"

using namespace StE::Graphics;

constexpr int linked_light_lists::lll_image_res_multiplier;

void linked_light_lists::resize(glm::ivec2 size) {
	size = size / lll_image_res_multiplier;

	if (size.x <= 0 || size.y <= 0)
		return;

	this->size = size;

	lll.commit_range(0, size.x * size.y * total_max_active_lights_per_frame);
	lll_heads = std::make_unique<Core::Texture2D>(gli::format::FORMAT_R32_UINT_PACK32, glm::ivec2{ size });
	lll_low_detail_heads = std::make_unique<Core::Texture2D>(gli::format::FORMAT_R32_UINT_PACK32, glm::ivec2{ size });
}

void linked_light_lists::bind_lll_buffer(bool low_detail) const {
	using namespace StE::Core;

	11_storage_idx = lll;

	if (low_detail)
		6_image_idx = (*lll_low_detail_heads)[0].with_access(Core::ImageAccessMode::Read);
	else
		6_image_idx = (*lll_heads)[0].with_access(Core::ImageAccessMode::Read);
}

void linked_light_lists::bind_readwrite_lll_buffers() const {
	using namespace StE::Core;

	11_storage_idx = lll;
	6_image_idx = (*lll_heads)[0].with_access(Core::ImageAccessMode::Write);
	7_image_idx = (*lll_low_detail_heads)[0].with_access(Core::ImageAccessMode::Write);
	7_storage_idx = lll_counter;
}
