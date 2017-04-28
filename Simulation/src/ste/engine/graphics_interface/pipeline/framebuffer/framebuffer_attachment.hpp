//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <image_layout.hpp>

#include <format.hpp>
#include <framebuffer_attachment_clear_value.hpp>
#include <attachment_load_op.hpp>
#include <attachment_store_op.hpp>

#include <image_view.hpp>
#include <vk_blend_op_descriptor.hpp>
#include <vk_render_pass_attachment.hpp>

namespace ste {
namespace gl {

struct framebuffer_attachment {
	const image_view_generic *output{ nullptr };

	image_layout initial_layout;
	image_layout layout;
	image_layout final_layout;

	attachment_load_op load_op;
	attachment_store_op store_op;

	framebuffer_attachment_clear_value clear_value;
	vk::vk_blend_op_descriptor blend_op;

	operator vk::vk_render_pass_attachment() const {
		return vk::vk_render_pass_attachment(static_cast<VkFormat>(output->get_format()),
											 static_cast<VkImageLayout>(initial_layout),
											 static_cast<VkImageLayout>(final_layout),
											 static_cast<VkAttachmentLoadOp>(load_op),
											 static_cast<VkAttachmentStoreOp>(store_op));
	}

	bool operator==(const framebuffer_attachment &rhs) const {
		return std::memcmp(this, &rhs, sizeof(rhs)) == 0;
	}
	bool operator!=(const framebuffer_attachment &rhs) const {
		return !(*this == rhs);
	}

	/**
	 *	@brief	Checks if the attachments are renderpass compatible
	 */
	bool renderpass_compatible(const framebuffer_attachment &rhs) const {
		bool format_compatible = 
			(!output && !rhs.output) || 
			(output && rhs.output && output->get_format() == rhs.output->get_format());
		bool layout_compatible =
			initial_layout == rhs.initial_layout &&
			layout == rhs.layout &&
			final_layout == rhs.final_layout;

		return format_compatible && layout_compatible;
	}

	/**
	*	@brief	Checks if the attachments are graphical pipeline settings compatible
	*/
	bool pipeline_settings_compatible(const framebuffer_attachment &rhs) const {
		return blend_op == rhs.blend_op;
	}

	/**
	*	@brief	Checks if the attachments are framebuffer compatible
	*/
	bool framebuffer_compatible(const framebuffer_attachment &rhs) const {
		return output == rhs.output &&
			clear_value == rhs.clear_value;
	}
};

/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially clear attachment content and store after render.
*/
framebuffer_attachment inline clear_store(const image_view_generic *output,
										  framebuffer_attachment_clear_value clear_value,
										  image_layout layout,
										  image_layout final_layout) {
	return {
		output,
		image_layout::undefined,
		layout,
		final_layout,
		attachment_load_op::clear,
		attachment_store_op::store,
		clear_value
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially clear attachment content and discard contents after render.
*/
framebuffer_attachment inline clear_discard(const image_view_generic *output,
											framebuffer_attachment_clear_value clear_value,
											image_layout layout,
											image_layout final_layout) {
	return {
		output,
		image_layout::undefined,
		layout,
		final_layout,
		attachment_load_op::clear,
		attachment_store_op::discard,
		clear_value
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially load attachment content and store after render.
*/
framebuffer_attachment inline load_store(const image_view_generic *output,
										 image_layout initial_layout,
										 image_layout layout,
										 image_layout final_layout) {
	return {
		output,
		initial_layout,
		layout,
		final_layout,
		attachment_load_op::load,
		attachment_store_op::store
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially load attachment content and discard contents after render.
*/
framebuffer_attachment inline load_discard(const image_view_generic *output,
										   image_layout initial_layout,
										   image_layout layout,
										   image_layout final_layout) {
	return {
		output,
		initial_layout,
		layout,
		final_layout,
		attachment_load_op::load,
		attachment_store_op::discard
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Ignore initial attachment content and store contents after render.
*/
framebuffer_attachment inline ignore_store(const image_view_generic *output,
										   image_layout layout,
										   image_layout final_layout) {
	return {
		output,
		image_layout::undefined,
		layout,
		final_layout,
		attachment_load_op::undefined,
		attachment_store_op::store
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Ignore initial attachment content and discard contents after render.
*/
framebuffer_attachment inline ignore_discard(const image_view_generic *output,
											 image_layout layout,
											 image_layout final_layout) {
	return {
		output,
		image_layout::undefined,
		layout,
		final_layout,
		attachment_load_op::undefined,
		attachment_store_op::discard
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially clear attachment content and store after render.
*/
framebuffer_attachment inline clear_store(const image_view_generic *output,
										  framebuffer_attachment_clear_value clear_value,
										  image_layout layout,
										  image_layout final_layout,
										  const vk::vk_blend_op_descriptor &blend_op) {
	return {
		output,
		image_layout::undefined,
		layout,
		final_layout,
		attachment_load_op::clear,
		attachment_store_op::store,
		clear_value,
		blend_op
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially clear attachment content and discard contents after render.
*/
framebuffer_attachment inline clear_discard(const image_view_generic *output,
											framebuffer_attachment_clear_value clear_value,
											image_layout layout,
											image_layout final_layout,
											const vk::vk_blend_op_descriptor &blend_op) {
	return {
		output,
		image_layout::undefined,
		layout,
		final_layout,
		attachment_load_op::clear,
		attachment_store_op::discard,
		clear_value,
		blend_op
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially load attachment content and store after render.
*/
framebuffer_attachment inline load_store(const image_view_generic *output,
										 image_layout initial_layout,
										 image_layout layout,
										 image_layout final_layout,
										 const vk::vk_blend_op_descriptor &blend_op) {
	return {
		output,
		initial_layout,
		layout,
		final_layout,
		attachment_load_op::load,
		attachment_store_op::store,
		framebuffer_attachment_clear_value(),
		blend_op
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Intially load attachment content and discard contents after render.
*/
framebuffer_attachment inline load_discard(const image_view_generic *output,
										   image_layout initial_layout,
										   image_layout layout,
										   image_layout final_layout,
										   const vk::vk_blend_op_descriptor &blend_op) {
	return {
		output,
		initial_layout,
		layout,
		final_layout,
		attachment_load_op::load,
		attachment_store_op::discard,
		framebuffer_attachment_clear_value(),
		blend_op
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Ignore initial attachment content and store contents after render.
*/
framebuffer_attachment inline ignore_store(const image_view_generic *output,
										   image_layout layout,
										   image_layout final_layout,
										   const vk::vk_blend_op_descriptor &blend_op) {
	return {
		output,
		image_layout::undefined,
		layout,
		final_layout,
		attachment_load_op::undefined,
		attachment_store_op::store,
		framebuffer_attachment_clear_value(),
		blend_op
	};
}
/**
*	@brief	Creates a framebuffer attachment with the following load-store operations:
*			Ignore initial attachment content and discard contents after render.
*/
framebuffer_attachment inline ignore_discard(const image_view_generic *output,
											 image_layout layout,
											 image_layout final_layout,
											 const vk::vk_blend_op_descriptor &blend_op) {
	return {
		output,
		image_layout::undefined,
		layout,
		final_layout,
		attachment_load_op::undefined,
		attachment_store_op::discard,
		framebuffer_attachment_clear_value(),
		blend_op
	};
}

}
}
