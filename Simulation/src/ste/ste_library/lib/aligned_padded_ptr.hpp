//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/allocator.hpp>

#include <aligned_padded_ptr.hpp>

namespace ste {
namespace lib {

namespace _detail {

template <typename T, std::size_t align>
static constexpr auto aligned_padded_ptr_size = std::max<std::size_t>(align, alignof(T));

}

template <typename T, std::size_t alignment = _detail::aligned_padded_ptr_size<T, 64>>
using aligned_padded_ptr = ::ste::aligned_padded_ptr<T, alignment, allocator<std::uint8_t, alignment>>;

}
}
