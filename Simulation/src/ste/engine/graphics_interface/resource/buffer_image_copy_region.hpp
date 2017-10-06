//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <format_rtti.hpp>

namespace ste {
namespace gl {

struct buffer_image_copy_region_t {
	std::size_t buffer_offset{ 0 };
	glm::i32vec3 image_offset{ 0 };

	std::uint32_t buffer_row_length{ 0 };
	std::uint32_t buffer_image_height{ 0 };

	format image_format;
	levels_t mip{ 0 };
	layers_t base_layer{ 0 };
	layers_t layers{ all_layers };

	glm::u32vec3 extent{ 1 };

	auto vk_descriptor(byte_t buffer_element_size) const {
		const VkBufferImageCopy c = {
			buffer_offset * static_cast<std::size_t>(buffer_element_size),
			buffer_row_length * static_cast<std::uint32_t>(buffer_element_size),
			buffer_image_height * static_cast<std::uint32_t>(buffer_element_size) * static_cast<std::uint32_t>(buffer_element_size),
			{
				static_cast<VkImageAspectFlags>(format_aspect(image_format)),
				static_cast<std::uint32_t>(mip),
				static_cast<std::uint32_t>(base_layer),
				static_cast<std::uint32_t>(layers)
			},
			{ image_offset.x, image_offset.y, image_offset.z },
			{ extent.x, extent.y, extent.z }
		};

		return c;
	}
};

}
}
