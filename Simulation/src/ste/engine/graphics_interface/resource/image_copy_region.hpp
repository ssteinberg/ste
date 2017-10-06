//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <format_rtti.hpp>

namespace ste {
namespace gl {

struct image_copy_region_t {
	glm::i32vec3 src_image_offset{ 0 };
	glm::i32vec3 dst_image_offset{ 0 };

	format src_image_format;
	levels_t src_mip{ 0 };
	layers_t src_base_layer{ 0 };
	layers_t src_layers{ all_layers };

	format dst_image_format;
	levels_t dst_mip{ 0 };
	layers_t dst_base_layer{ 0 };
	layers_t dst_layers{ all_layers };

	glm::u32vec3 extent{ 1 };

	auto vk_descriptor() const {
		const VkImageCopy c = {
			{
				static_cast<VkImageAspectFlags>(format_aspect(src_image_format)),
				static_cast<std::uint32_t>(src_mip),
				static_cast<std::uint32_t>(src_base_layer),
				static_cast<std::uint32_t>(src_layers)
			},
			{ src_image_offset.x, src_image_offset.y, src_image_offset.z },
			{
				static_cast<VkImageAspectFlags>(format_aspect(dst_image_format)),
				static_cast<std::uint32_t>(dst_mip),
				static_cast<std::uint32_t>(dst_base_layer),
				static_cast<std::uint32_t>(dst_layers)
			},
			{ dst_image_offset.x, dst_image_offset.y, dst_image_offset.z },
			{ extent.x, extent.y, extent.z }
		};

		return c;
	}
};

}
}
