//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_image_base.hpp>
#include <image_memory_barrier.hpp>

namespace StE {
namespace GL {

auto inline image_layout_transform_barrier(const device_image_base &image,
										   VkAccessFlags src_access,
										   VkImageLayout src_layout,
										   VkAccessFlags dst_access,
										   VkImageLayout dst_layout) {
	return image_memory_barrier(image,
								src_layout,
								dst_layout,
								src_access,
								dst_access);
}

}
}
