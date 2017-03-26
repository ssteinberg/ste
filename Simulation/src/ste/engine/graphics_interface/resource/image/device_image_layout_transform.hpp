//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_image.hpp>

namespace StE {
namespace GL {

template <int dimensions, class allocation_policy>
auto image_layout_transform_barrier(const device_image<dimensions, allocation_policy> &image,
									VkAccessFlags src_access,
									VkImageLayout src_layout,
									VkAccessFlags dst_access,
									VkImageLayout dst_layout,
									bool depth = false) {
	return image_memory_barrier(image,
								src_layout,
								dst_layout,
								src_access,
								dst_access,
								depth);
}

}
}
