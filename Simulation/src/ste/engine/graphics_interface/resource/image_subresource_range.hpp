//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <format_rtti.hpp>

namespace ste {
namespace gl {

struct image_subresource_range {
	format image_format;
	levels_t base_mip{ 0 };
	levels_t mips{ all_mips };
	layers_t base_layer{ 0 };
	layers_t layers{ all_layers };

	auto vk_descriptor() const {
		const VkImageSubresourceRange c = {
			static_cast<VkImageAspectFlags>(format_aspect(image_format)),
			static_cast<std::uint32_t>(base_mip),
			static_cast<std::uint32_t>(mips),
			static_cast<std::uint32_t>(base_layer),
			static_cast<std::uint32_t>(layers)
		};

		return c;
	}
};

}
}
