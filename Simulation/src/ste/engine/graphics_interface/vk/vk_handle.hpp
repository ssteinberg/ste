//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace gl {

namespace vk {

namespace _detail {

template <int ptr_size>
struct vk_null_handle {};
template <>
struct vk_null_handle<4> {
	static constexpr std::uint64_t value = 0;
};
template <>
struct vk_null_handle<8> {
	static constexpr auto value = nullptr;
};

template <int ptr_size>
struct vk_handle {};
template <>
struct vk_handle<4> {
	std::uint64_t handle;
	vk_handle(std::uint64_t handle) : handle(handle) {}
	operator std::uint64_t() const { return handle; }
};
template <>
struct vk_handle<8> {
	std::uint64_t handle;
	vk_handle(const void* const handle) : handle(reinterpret_cast<std::uint64_t>(handle)) {}
	operator std::uint64_t() const { return handle; }
};

}

static constexpr auto vk_null_handle = _detail::vk_null_handle<sizeof(void*)>::value;

using vk_handle = _detail::vk_handle<sizeof(void*)>;

}

}
}
