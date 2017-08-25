//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <framebuffer_layout.hpp>
#include <framebuffer_attachment.hpp>
#include <framebuffer_bind_point.hpp>
#include <framebuffer_exceptions.hpp>

#include <vk_render_pass.hpp>
#include <vk_framebuffer.hpp>

#include <pipeline_layout_attachment_location.hpp>

#include <rect.hpp>
#include <depth_range.hpp>

#include <lib/flat_map.hpp>
#include <optional.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

/**
*	@brief	A pipeline framebuffer object
*/
class framebuffer : public allow_type_decay<framebuffer, vk::vk_framebuffer<>> {
public:
	using attachment_map_t = lib::flat_map<pipeline_layout_attachment_location, framebuffer_attachment>;

private:
	alias<const ste_context> ctx;

	framebuffer_layout layout;
	attachment_map_t attachments;

	glm::u32vec2 fb_extent;
	depth_range depth;

	vk::vk_render_pass<> compatible_renderpass;
	optional<vk::vk_framebuffer<>> fb;
	optional<lib::vector<VkClearValue>> clear_values;

private:
	void attach(pipeline_layout_attachment_location location, framebuffer_attachment &&attachment) {
		// Validate
		{
			auto layout_it = layout.find(location);
			if (layout_it == layout.end()) {
				// Location doesn't exist in layout
				throw framebuffer_invalid_attachment_location_exception("Attachment location doesn't exist in framebuffer layout");
			}

			if (layout_it->second.image_format != attachment.get_attachment().get_format()) {
				// Formats do not match
				throw framebuffer_attachment_format_mismatch_exception("Attachment format doesn't match layout's format");
			}

			if (layout_it->second.clears_on_load() && !attachment.has_explicit_clear_value()) {
				// Expected a clear value as attachment layout defines a clear on load operation.
				throw framebuffer_attachment_mismatch_exception("Layout has clear load operation but no clear value specified");
			}
		}

		bool invalidate_framebuffer = true;

		// Insert or overwrite attachment
		auto it = attachments.find(location);
		if (it != attachments.end()) {
			// Anything modified?
			if (it->second == attachment)
				return;

			// Was attachment modified?
			if (it->second.get_attachment().get_image_view_handle() == attachment.get_attachment().get_image_view_handle())
				invalidate_framebuffer = false;

			it->second = std::move(attachment);
		}
		else {
			attachments.insert(it, std::make_pair(location, std::move(attachment)));
		}

		if (!fb) {
			if (attachments.size() == layout.size()) {
				// Create framebuffer as early as possible the very first time we have enough attachments
				update();
			}
		}
		else {
			// Framebuffer was modified.
			// Recreate resources on next update
			if (invalidate_framebuffer)
				fb = none;
			clear_values = none;
		}
	}

	auto create_clear_values() {
		lib::vector<VkClearValue> ret;
		ret.reserve(attachments.size());

		auto it = attachments.begin();
		auto layout_it = layout.begin();
		for (auto it_next = it; it_next != attachments.end(); ++it_next, ++layout_it) {
			assert(layout_it != layout.end());
			assert(layout_it->first == it_next->first);

			if (layout_it->second.load_op == attachment_load_op::clear) {
				for (; it != it_next; ++it)
					ret.push_back(VkClearValue{});
				ret.push_back(it_next->second.get_vk_clear_value());
			}
		}

		return ret;
	}

	auto create_framebuffer() {
		lib::vector<VkImageView> image_view_handles;
		image_view_handles.reserve(attachments.size());
		for (auto &a : attachments) {
			image_view_handles.push_back(a.second.get_attachment().get_image_view_handle());
		}

		// Create Vulkan framebuffer
		return vk::vk_framebuffer<>(ctx.get().device(),
									compatible_renderpass,
									image_view_handles,
									fb_extent);
	}

private:
	using bind_point_t = framebuffer_bind_point<framebuffer, &framebuffer::attach>;

	friend class bind_point_t;

public:
	framebuffer(const ste_context &ctx,
				framebuffer_layout &&layout,
				const glm::u32vec2 &extent,
				const depth_range &depth = depth_range::one_to_zero())
		: ctx(ctx),
		layout(std::move(layout)), 
		fb_extent(extent),
		depth(depth),
		compatible_renderpass(this->layout.create_compatible_renderpass(ctx))
	{}
	framebuffer(const ste_context &ctx,
				const framebuffer_layout &layout,
				const glm::u32vec2 &extent,
				const depth_range &depth = depth_range::one_to_zero())
		: framebuffer(ctx,
					  framebuffer_layout(layout),
					  extent,
					  depth)
	{}

	framebuffer(framebuffer&&) = default;
	framebuffer&operator=(framebuffer&&) = default;

	/**
	 *	@brief	Creates a bind point to the framebuffer.
	 *	
	 *	@param	location		Attachment location
	 */
	auto operator[](const pipeline_layout_attachment_location &location) {
		return bind_point_t(location, this);
	}

	/**
	 *	@brief	Updates the framebuffer. Should be called before using the framebuffer object.
	 */
	void update() {
		if (!fb) {
			// Recreate framebuffer
			fb = create_framebuffer();
		}
		if (!clear_values) {
			// Recreate clear values array
			clear_values = create_clear_values();
		}
	}

	/**
	 *	@brief	Returns the Vulkan framebuffer object
	 */
	const auto& get() const {
		assert(fb && "update() not called?");
		return fb.get();
	}

	/**
	*	@brief	Returns the framebuffer layout
	*/
	const auto& get_layout() const {
		return layout;
	}

	/**
	*	@brief	Returns the framebuffer attachment clear values array
	*/
	const auto& get_fb_clearvalues() const {
		assert(fb && "clear_values() not called?");
		return clear_values.get();
	}

	/**
	*	@brief	Returns the framebuffer's extent
	*/
	auto& extent() const { return fb_extent; }

	/**
	*	@brief	Returns the framebuffer's depth range
	*/
	auto& get_depth_range() const { return depth; }

	auto begin() const { return attachments.begin(); }
	auto end() const { return attachments.end(); }

	auto size() const {
		return attachments.size();
	}
};

}
}
