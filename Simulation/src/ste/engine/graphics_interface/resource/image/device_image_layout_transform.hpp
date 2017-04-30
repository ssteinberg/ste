//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_image_base.hpp>
#include <image_memory_barrier.hpp>
#include <image_layout.hpp>
#include <access_flags.hpp>

namespace ste {
namespace gl {

/**
 *	@brief	Creates an image memory barrier to transform the image layout.
 */
auto inline image_layout_transform_barrier(const device_image_base &image,
										   access_flags src_access,
										   image_layout src_layout,
										   access_flags dst_access,
										   image_layout dst_layout) {
	return image_memory_barrier(image,
								src_layout,
								dst_layout,
								src_access,
								dst_access);
}

/**
*	@brief	Creates an image memory barrier to transform the image layout.
*			Deduces the access flags based on source and destination layouts. Layouts must no be undefined, preinitialized or general.
*			
 *	@throws	ste_engine_exception		If image layout is undefined, preinitialized or general.
*/
auto inline image_layout_transform_barrier(const device_image_base &image,
										   image_layout src_layout,
										   image_layout dst_layout) {
	return image_memory_barrier(image,
								src_layout,
								dst_layout,
								access_flags_for_image_layout(src_layout),
								access_flags_for_image_layout(dst_layout));
}

}
}
