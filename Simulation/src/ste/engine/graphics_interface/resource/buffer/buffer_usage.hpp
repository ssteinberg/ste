//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

enum class buffer_usage : std::uint32_t {
	transfer_src = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	transfer_dst = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	uniform_texel_buffer = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
	storage_texel_buffer = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
	uniform_buffer = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	storage_buffer = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
	index_buffer = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	vertex_buffer = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	indirect_buffer = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
};

constexpr auto operator|(const buffer_usage &lhs, const buffer_usage &rhs) {
	using T = std::underlying_type_t<buffer_usage>;
	return static_cast<buffer_usage>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
constexpr auto operator&(const buffer_usage &lhs, const buffer_usage &rhs) {
	using T = std::underlying_type_t<buffer_usage>;
	return static_cast<buffer_usage>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
constexpr auto operator^(const buffer_usage &lhs, const buffer_usage &rhs) {
	using T = std::underlying_type_t<buffer_usage>;
	return static_cast<buffer_usage>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

}
}
