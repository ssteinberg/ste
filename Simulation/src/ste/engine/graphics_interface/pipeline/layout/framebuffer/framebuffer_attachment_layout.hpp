//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_render_pass_attachment.hpp>

#include <format.hpp>
#include <format_rtti.hpp>
#include <image_layout.hpp>
#include <attachment_load_op.hpp>
#include <attachment_store_op.hpp>

#include <blend_operation.hpp>

namespace ste {
namespace gl {

namespace _detail {

auto inline attachment_layout_for_format(format f) {
	return format_is_depth(f) ?
		image_layout::depth_stencil_attachment_optimal : 
		image_layout::color_attachment_optimal;
}

}

struct framebuffer_attachment_layout {
	format image_format;

	image_layout initial_layout;
	image_layout layout;
	image_layout final_layout;

	attachment_load_op load_op;
	attachment_store_op store_op;

	blend_operation blend;

	framebuffer_attachment_layout() = default;

	operator vk::vk_render_pass_attachment() const {
		return vk::vk_render_pass_attachment(static_cast<VkFormat>(image_format),
											 static_cast<VkImageLayout>(initial_layout),
											 static_cast<VkImageLayout>(final_layout),
											 static_cast<VkAttachmentLoadOp>(load_op),
											 static_cast<VkAttachmentStoreOp>(store_op));
	}

	bool operator==(const framebuffer_attachment_layout &rhs) const {
		return std::memcmp(this, &rhs, sizeof(rhs)) == 0;
	}
	bool operator!=(const framebuffer_attachment_layout &rhs) const {
		return !(*this == rhs);
	}

	/**
	 *	@brief	Returns true if the attachment layout specifies clearing the attachment on load. Those layouts require clear value
	 *			to be specified by the pipeline.
	 */
	bool clears_on_load() const { return load_op == attachment_load_op::clear; }

	/**
	 *	@brief	Checks if the attachments are renderpass compatible
	 */
	bool renderpass_compatible(const framebuffer_attachment_layout &rhs) const {
		bool format_compatible = image_format == rhs.image_format;
		bool layout_compatible =
			initial_layout == rhs.initial_layout &&
			layout == rhs.layout &&
			final_layout == rhs.final_layout;

		return format_compatible && layout_compatible;
	}

	/**
	*	@brief	Checks if the attachments are graphical pipeline settings compatible
	*/
	bool pipeline_settings_compatible(const framebuffer_attachment_layout &rhs) const {
		return blend == rhs.blend;
	}
};

/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially clear attachment content and store after render.
*/
framebuffer_attachment_layout inline clear_store(format image_format,
												 image_layout final_layout) {
	return {
		image_format,
		image_layout::undefined,
		_detail::attachment_layout_for_format(image_format),
		final_layout,
		attachment_load_op::clear,
		attachment_store_op::store,
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially clear attachment content and discard contents after render.
*/
framebuffer_attachment_layout inline clear_discard(format image_format,
												   image_layout final_layout) {
	return {
		image_format,
		image_layout::undefined,
		_detail::attachment_layout_for_format(image_format),
		final_layout,
		attachment_load_op::clear,
		attachment_store_op::discard,
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially load attachment content and store after render.
*/
framebuffer_attachment_layout inline load_store(format image_format,
												image_layout initial_layout,
												image_layout final_layout) {
	return {
		image_format,
		initial_layout,
		_detail::attachment_layout_for_format(image_format),
		final_layout,
		attachment_load_op::load,
		attachment_store_op::store
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially load attachment content and discard contents after render.
*/
framebuffer_attachment_layout inline load_discard(format image_format,
												  image_layout initial_layout,
												  image_layout final_layout) {
	return {
		image_format,
		initial_layout,
		_detail::attachment_layout_for_format(image_format),
		final_layout,
		attachment_load_op::load,
		attachment_store_op::discard
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Ignore initial attachment content and store contents after render.
*/
framebuffer_attachment_layout inline ignore_store(format image_format,
												  image_layout final_layout) {
	return {
		image_format,
		image_layout::undefined,
		_detail::attachment_layout_for_format(image_format),
		final_layout,
		attachment_load_op::undefined,
		attachment_store_op::store
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Ignore initial attachment content and discard contents after render.
*/
framebuffer_attachment_layout inline ignore_discard(format image_format,
													image_layout final_layout) {
	return {
		image_format,
		image_layout::undefined,
		_detail::attachment_layout_for_format(image_format),
		final_layout,
		attachment_load_op::undefined,
		attachment_store_op::discard
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially clear attachment content and store after render.
*/
framebuffer_attachment_layout inline clear_store(format image_format,
												 image_layout final_layout,
												 const blend_operation &blend) {
	return {
		image_format,
		image_layout::undefined,
		_detail::attachment_layout_for_format(image_format),
		final_layout,
		attachment_load_op::clear,
		attachment_store_op::store,
		blend
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially clear attachment content and discard contents after render.
*/
framebuffer_attachment_layout inline clear_discard(format image_format,
												   image_layout final_layout,
												   const blend_operation &blend) {
	return {
		image_format,
		image_layout::undefined,
		_detail::attachment_layout_for_format(image_format),
		final_layout,
		attachment_load_op::clear,
		attachment_store_op::discard,
		blend
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially load attachment content and store after render.
*/
framebuffer_attachment_layout inline load_store(format image_format,
												image_layout initial_layout,
												image_layout final_layout,
												const blend_operation &blend) {
	return {
		image_format,
		initial_layout,
		_detail::attachment_layout_for_format(image_format),
		final_layout,
		attachment_load_op::load,
		attachment_store_op::store,
		blend
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially load attachment content and discard contents after render.
*/
framebuffer_attachment_layout inline load_discard(format image_format,
												  image_layout initial_layout,
												  image_layout final_layout,
												  const blend_operation &blend) {
	return {
		image_format,
		initial_layout,
		_detail::attachment_layout_for_format(image_format),
		final_layout,
		attachment_load_op::load,
		attachment_store_op::discard,
		blend
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Ignore initial attachment content and store contents after render.
*/
framebuffer_attachment_layout inline ignore_store(format image_format,
												  image_layout final_layout,
												  const blend_operation &blend) {
	return {
		image_format,
		image_layout::undefined,
		_detail::attachment_layout_for_format(image_format),
		final_layout,
		attachment_load_op::undefined,
		attachment_store_op::store,
		blend
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Ignore initial attachment content and discard contents after render.
*/
framebuffer_attachment_layout inline ignore_discard(format image_format,
													image_layout final_layout,
													const blend_operation &blend) {
	return {
		image_format,
		image_layout::undefined,
		_detail::attachment_layout_for_format(image_format),
		final_layout,
		attachment_load_op::undefined,
		attachment_store_op::discard,
		blend
	};
}

}
}
