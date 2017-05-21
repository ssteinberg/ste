//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <image_memory_barrier.hpp>
#include <image_layout.hpp>
#include <access_flags.hpp>

#include <device_image_base.hpp>
#include <texture.hpp>

namespace ste {
namespace gl {

/**
 *	@brief	Creates an image memory barrier to transform the image layout.
 */
auto inline image_layout_transform_barrier(const device_image_base &image,
										   image_layout src_layout,
										   image_layout dst_layout,
										   access_flags src_access,
										   access_flags dst_access) {
	return image_memory_barrier(image,
								src_layout,
								dst_layout,
								src_access,
								dst_access);
}
/**
 *	@brief	Creates an image memory barrier to transform the image layout.
 */
auto inline image_layout_transform_barrier(const texture_generic &texture,
										   image_layout src_layout,
										   image_layout dst_layout,
										   access_flags src_access,
										   access_flags dst_access) {
	return image_layout_transform_barrier(texture.get_image(),
										  src_layout,
										  dst_layout,
										  src_access,
										  dst_access);
}

/**
*	@brief	Creates an image memory barrier to transform the image layout.
*			Deduces the access flags based on source and destination layouts. Layouts must no be undefined or preinitialized.
*
*	@throws	ste_engine_exception		If image layout is undefined or preinitialized.
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
/**
*	@brief	Creates an image memory barrier to transform the image layout.
*			Deduces the access flags based on source and destination layouts. Layouts must no be undefined or preinitialized.
*
*	@throws	ste_engine_exception		If image layout is undefined or preinitialized.
*/
auto inline image_layout_transform_barrier(const texture_generic &texture,
										   image_layout src_layout,
										   image_layout dst_layout) {
	return image_layout_transform_barrier(texture.get_image(),
										  src_layout,
										  dst_layout);
}

}
}
