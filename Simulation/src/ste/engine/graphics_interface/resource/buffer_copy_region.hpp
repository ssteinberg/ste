//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

struct buffer_copy_region_t {
	std::size_t src_buffer_offset{ 0 };
	std::size_t dst_buffer_offset{ 0 };
	byte_t bytes;

	auto vk_descriptor(byte_t src_buffer_element_size, byte_t dst_buffer_element_size) const {
		const VkBufferCopy c = {
			src_buffer_offset * static_cast<std::size_t>(src_buffer_element_size),
			dst_buffer_offset * static_cast<std::size_t>(dst_buffer_element_size),
			static_cast<std::size_t>(bytes)
		};

		return c;
	}
};

}
}
