//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <framebuffer_attachment_layout.hpp>
#include <framebuffer_exceptions.hpp>
#include <pipeline_layout_attachment_location.hpp>

#include <boost/container/flat_map.hpp>
#include <initializer_list>

namespace ste {
namespace gl {

/**
*	@brief	A collection of framebuffer attachment layouts
*/
class framebuffer_layout {
public:
	using attachment_map_t = boost::container::flat_map<pipeline_layout_attachment_location, framebuffer_attachment_layout>;

private:
	glm::u32vec2 fb_extent;
	attachment_map_t attachments;

public:
	framebuffer_layout(glm::u32vec2 extent) : fb_extent(extent) {}
	framebuffer_layout(glm::u32vec2 extent,
					   const std::initializer_list<attachment_map_t::value_type> &il)
		: fb_extent(extent), attachments(il) {}

	framebuffer_layout(framebuffer_layout&&) = default;
	framebuffer_layout &operator=(framebuffer_layout&&) = default;
	framebuffer_layout(const framebuffer_layout&) = default;
	framebuffer_layout &operator=(const framebuffer_layout&) = default;

	auto& operator[](const pipeline_layout_attachment_location &location) { return attachments[location]; }

	void erase(const pipeline_layout_attachment_location &location) { attachments.erase(location); }
	void erase(attachment_map_t::iterator it) { attachments.erase(it); }

	auto find(const pipeline_layout_attachment_location &location) const { return attachments.find(location); }

	auto& get() const { return attachments; }
	auto begin() const { return attachments.begin(); }
	auto end() const { return attachments.end(); }

	auto size() const {
		return attachments.size();
	}

	/**
	 *	@brief	Checks if the framebuffer layouts are compatible.
	 *			Two layouts are compatible if for each location both layouts are either unbound or bind attachments with 
	 *			identical format.
	 *			https://www.khronos.org/registry/vulkan/specs/1.0-extensions/html/vkspec.html#renderpass-compatibility
	 */
	bool compatible(const framebuffer_layout &rhs) const {
		if (attachments.size() != rhs.attachments.size())
			return false;

		// Containers are ordered, so just iterate
		for (auto it_lhs = begin(), it_rhs = rhs.begin(); it_lhs != end(); ++it_lhs, ++it_rhs) {
			// Check that locations match
			if (it_rhs->first || it_lhs->first)
				return false;

			const auto &a1 = it_lhs->second;
			const auto &a2 = it_rhs->second;
			// Check that formats match
			if (a1.image_format != a2.image_format)
				return false;
		}

		return true;
	}

	auto extent() const {
		return fb_extent;
	}

	auto create_compatible_renderpass(const ste_context &ctx) const {
		// Sort attachments
		std::vector<vk::vk_render_pass_attachment> vk_attachments;
		std::vector<VkAttachmentReference> color;
		optional<VkAttachmentReference> depth;

		for (auto &a : *this) {
			const framebuffer_attachment_layout &attachment = a.second;
			vk_attachments.push_back(attachment);

			bool is_depth_attachment = format_is_depth(attachment.image_format);
			if (is_depth_attachment && depth) {
				// Only one depth attachment per pipeline
				throw framebuffer_layout_multiple_depth_attachments_exception("Up to one depth attachment allowed per framebuffer");
			}

			auto vk_attachment_ref = VkAttachmentReference{ vk_attachments.size() - 1, static_cast<VkImageLayout>(attachment.layout) };
			if (is_depth_attachment)
				depth = vk_attachment_ref;
			else
				color.push_back(vk_attachment_ref);
		}

		// Create subpass descriptor
		std::vector<vk::vk_render_pass_subpass_descriptor> subpasses;
		depth ?
			subpasses.emplace_back(color, depth.get()) :
			subpasses.emplace_back(color);

		// Create Vulkan renderpass
		return vk::vk_render_pass(ctx.device(),
								  vk_attachments,
								  subpasses);
	}
};

}
}
