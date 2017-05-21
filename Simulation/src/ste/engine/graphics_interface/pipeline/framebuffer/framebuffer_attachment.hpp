//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <image_view.hpp>
#include <framebuffer_attachment_clear_value.hpp>

namespace ste {
namespace gl {

/**
*	@brief	A pipeline framebuffer attachment
*/
class framebuffer_attachment {
private:
	const image_view_generic *attachment;
	optional<framebuffer_attachment_clear_value> clear_value;

public:
	/**
	 *	@brief	Attachments without a clear value
	 */
	framebuffer_attachment(const image_view_generic &attachment)
		: attachment(&attachment)
	{}
	/**
	*	@brief	Attachments with a clear value
	*/
	framebuffer_attachment(const image_view_generic &attachment,
						   const framebuffer_attachment_clear_value &clear_value)
		: attachment(&attachment),
		clear_value(clear_value)
	{}

	framebuffer_attachment(framebuffer_attachment&&) = default;
	framebuffer_attachment &operator=(framebuffer_attachment&&) = default;

	bool operator==(const framebuffer_attachment &rhs) const {
		return attachment->get_image_view_handle() == rhs.attachment->get_image_view_handle() &&
			clear_value == rhs.clear_value;
	}
	bool operator!=(const framebuffer_attachment &rhs) const {
		return !(*this == rhs);
	}

	const auto& get_attachment() const { return *attachment; }

	/**
	 *	@brief	Returns true if the attachment has explicitly specified a clear value, false otherwise.
	 */
	bool has_explicit_clear_value() const { return !!clear_value; }

	/**
	*	@brief	Returns the Vulkan clear value descriptor
	*/
	VkClearValue get_vk_clear_value() const {
		return clear_value ?
			clear_value.get().vk_clear_value(attachment->get_format()) :
			VkClearValue{};
	}
};

}
}
