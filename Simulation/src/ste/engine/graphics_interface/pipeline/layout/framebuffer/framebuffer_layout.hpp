//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <framebuffer_attachment_layout.hpp>
#include <framebuffer_exceptions.hpp>
#include <pipeline_layout_attachment_location.hpp>

#include <lib/flat_map.hpp>
#include <initializer_list>
#include <optional.hpp>

namespace ste {
namespace gl {

/**
*	@brief	A collection of framebuffer attachment layouts
*/
class framebuffer_layout {
public:
	using attachment_map_t = lib::flat_map<pipeline_layout_attachment_location, framebuffer_attachment_layout>;

private:
	attachment_map_t attachments;

	mutable optional<std::uint32_t> highest_index_of_attachment_with_load_op;

public:
	framebuffer_layout() = default;
	framebuffer_layout(const std::initializer_list<attachment_map_t::value_type> &il)
		: attachments(il) {}

	framebuffer_layout(framebuffer_layout&&) = default;
	framebuffer_layout &operator=(framebuffer_layout&&) = default;
	framebuffer_layout(const framebuffer_layout&) = default;
	framebuffer_layout &operator=(const framebuffer_layout&) = default;

	auto& operator[](const pipeline_layout_attachment_location &location) {
		highest_index_of_attachment_with_load_op = none;
		return attachments[location];
	}

	void erase(const pipeline_layout_attachment_location &location) {
		highest_index_of_attachment_with_load_op = none;
		attachments.erase(location);
	}
	void erase(attachment_map_t::iterator it) {
		highest_index_of_attachment_with_load_op = none;
		attachments.erase(it);
	}

	auto find(const pipeline_layout_attachment_location &location) const { return attachments.find(location); }

	auto& get() const { return attachments; }
	auto begin() const { return attachments.begin(); }
	auto end() const { return attachments.end(); }

	auto size() const {
		return attachments.size();
	}

	auto get_highest_index_of_attachment_with_load_op() const {
		if (!highest_index_of_attachment_with_load_op) {
			highest_index_of_attachment_with_load_op = 0;
			for (std::uint32_t i = 0; i < size(); ++i)
				if ((begin() + i)->second.load_op == attachment_load_op::clear)
					highest_index_of_attachment_with_load_op = i + 1;
		}

		return highest_index_of_attachment_with_load_op.get();
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
			if (it_rhs->first != it_lhs->first)
				return false;

			const auto &a1 = it_lhs->second;
			const auto &a2 = it_rhs->second;
			// Check that formats match
			if (a1.image_format != a2.image_format)
				return false;
		}

		return true;
	}

	auto create_compatible_renderpass(const ste_context &ctx) const {
		// Sort attachments
		lib::vector<vk::vk_render_pass_attachment> vk_attachments;
		lib::vector<VkAttachmentReference> color;
		optional<VkAttachmentReference> depth;

		for (auto &a : *this) {
			const framebuffer_attachment_layout &attachment = a.second;
			vk_attachments.push_back(attachment);

			bool is_depth_attachment = format_is_depth(attachment.image_format);
			if (is_depth_attachment && depth) {
				// Only one depth attachment per pipeline
				throw framebuffer_layout_multiple_depth_attachments_exception("Up to one depth attachment allowed per framebuffer");
			}

			// Pad to attachment location
			if (!is_depth_attachment) {
				while (color.size() > a.first) color.push_back(VkAttachmentReference{ VK_ATTACHMENT_UNUSED, static_cast<VkImageLayout>(0) });
				assert(color.size() == a.first);
			}

			auto vk_attachment_ref = VkAttachmentReference{ 
				static_cast<std::uint32_t>(vk_attachments.size() - 1),
				static_cast<VkImageLayout>(attachment.layout)
			};
			if (is_depth_attachment)
				depth = vk_attachment_ref;
			else
				color.push_back(vk_attachment_ref);
		}

		// Create subpass descriptor
		lib::vector<vk::vk_render_pass_subpass_descriptor> subpasses;
		depth ?
			subpasses.emplace_back(color, depth.get()) :
			subpasses.emplace_back(color);

		// Create Vulkan renderpass
		return vk::vk_render_pass<>(ctx.device(),
									vk_attachments,
									subpasses);
	}
};

}
}
