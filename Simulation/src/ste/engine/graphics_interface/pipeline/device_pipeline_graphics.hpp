//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_pipeline.hpp>
#include <pipeline_framebuffer.hpp>
#include <pipeline_framebuffer_bind_point.hpp>
#include <device_pipeline_exceptions.hpp>

#include <pipeline_vertex_input_bindings_collection.hpp>

#include <vk_render_pass.hpp>
#include <vk_framebuffer.hpp>
#include <vk_pipeline_graphics.hpp>

#include <cmd_begin_render_pass.hpp>
#include <cmd_end_render_pass.hpp>
#include <cmd_bind_pipeline.hpp>

#include <optional.hpp>
#include <string>
#include <format_rtti.hpp>
#include <vector>

namespace ste {
namespace gl {

class device_pipeline_graphics : public device_pipeline {
	using Base = device_pipeline;

	friend class pipeline_auditor_graphics;

	struct ctor {};

private:
	class fb_ref {
	private:
		device_pipeline_graphics *owner;

	public:
		fb_ref() = default;
		fb_ref(device_pipeline_graphics *o) : owner(o) {}

		auto operator[](const std::string &name) {
			// Get location of attachment using layout's map
			const auto &attachments_map = owner->layout.attachments();
			auto it = attachments_map.find(name);
			if (it == attachments_map.end()) {
				// No such name exists
				throw device_pipeline_unrecognized_variable_name_exception("Attachment with provided name doesn't exist in pipeline layout");
			}

			auto location = it->second.location();
			optional<std::reference_wrapper<const framebuffer_attachment>> attachment;

			// If found, get the attachment at location (if any)
			auto ait = owner->fb.find(location);
			if (ait != owner->fb.end()) {
				const auto &a = ait->second;
				if (a)
					attachment = std::ref(a.get());
			}

			// Create attachment binder
			return fb_bind_point(location,
								 std::move(attachment),
								 owner);
		}
	};

private:
	device_pipeline_graphics_configurations pipeline_settings;
	pipeline_vertex_input_bindings_collection::pipeline_vertex_input_bindings_descriptor vertex_input_descriptor;

	pipeline_framebuffer fb;

	glm::u32vec2 extent;
	optional<vk::vk_framebuffer> device_framebuffer;
	optional<vk::vk_render_pass> device_renderpass;
	optional<vk::vk_pipeline_graphics> graphics_pipeline;

	optional<std::vector<VkClearValue>> clear_values;

private:
	void invalidate_framebuffer() {
		device_framebuffer = none;
		clear_values = none;
	}
	void invalidate_renderpass() {
		device_renderpass = none;
	}
	void invalidate_pipeline() {
		graphics_pipeline = none;
	}
	void invalidate_all() {
		device_framebuffer = none;
		device_renderpass = none;
		graphics_pipeline = none;
		clear_values = none;
	}

	// Framebuffer attachment binding
	void bind_fb_attachment(pipeline_layout_attachment_location location,
							framebuffer_attachment &&attachment) {
		auto it = fb.find(location);
		if (it == fb.end()) {
			// Previuosly unspecified in framebuffer
			fb.insert(it, std::make_pair(location, std::move(attachment)));
			invalidate_all();

			// If all attachment have now been declared, create the pipeline and framebuffer
			create_pipeline();
			create_framebuffer();
		}
		else {
			auto &a = it->second;
			if (!a) {
				// Previously unbound
				invalidate_all();
			}
			else {
				// Updating an attachment binding
				auto &prev = a.get();

				if (!attachment.framebuffer_compatible(prev))
					invalidate_framebuffer();
				if (!attachment.renderpass_compatible(prev))
					invalidate_renderpass();
				if (!attachment.pipeline_settings_compatible(prev))
					invalidate_pipeline();
			}

			it->second = std::move(attachment);
		}
	}

	using fb_bind_point = pipeline_framebuffer_bind_point<device_pipeline_graphics, &device_pipeline_graphics::bind_fb_attachment>;
	friend class fb_bind_point;

private:
	// Creates the Vulkan renderpass object
	void create_renderpass() {
		// Sort attachments
		std::vector<vk::vk_render_pass_attachment> vk_attachments;
		std::vector<VkAttachmentReference> color;
		optional<VkAttachmentReference> depth;

		for (auto &a : fb) {
			// a is an optional attachment
			if (!a.second)
				continue;

			const framebuffer_attachment &attachment = a.second.get();
			vk_attachments.push_back(attachment);

			bool is_depth_attachment = vk_format_is_depth(attachment.output->get_format());
			if (is_depth_attachment && depth) {
				// Only one depth attachment per pipeline
				throw device_pipeline_multiple_depth_attachments_exception("Up to one depth attachment allowed per framebuffer");
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
		device_renderpass.emplace(ctx.device(),
								  vk_attachments,
								  subpasses);
	}

	// Creates the graphics pipeline object
	void create_pipeline() {
		if (!device_renderpass)
			create_renderpass();

		// Vulkan shader stage descriptors
		auto shader_stage_descriptors = layout.shader_stage_descriptors();

		// Blend operation descriptor for each attachment
		std::vector<vk::vk_blend_op_descriptor> attachment_blend_ops;
		for (auto &a : fb) {
			// a is an optional attachment
			if (!a.second)
				continue;

			// Blend operation is only applicable for color attachments
			const framebuffer_attachment &attachment = a.second.get();
			bool is_depth_attachment = vk_format_is_depth(attachment.output->get_format());
			if (is_depth_attachment)
				continue;

			attachment_blend_ops.push_back(attachment.blend_op);
		}

		// Create the graphics pipeline object
		VkViewport viewport = { 
			pipeline_settings.viewport.origin.x,
			pipeline_settings.viewport.origin.y,
			pipeline_settings.viewport.size.x,
			pipeline_settings.viewport.size.y,
			pipeline_settings.depth.min_depth,
			pipeline_settings.depth.max_depth,
		};
		VkRect2D scissor = {
			{ pipeline_settings.scissor.origin.x,pipeline_settings.scissor.origin.y },
			{ pipeline_settings.scissor.size.x,pipeline_settings.scissor.size.y },
		};
		graphics_pipeline.emplace(ctx.device(),
								  shader_stage_descriptors,
								  layout,
								  device_renderpass.get(),
								  0,
								  viewport,
								  scissor,
								  vertex_input_descriptor.vertex_input_binding_descriptors,
								  vertex_input_descriptor.vertex_input_attribute_descriptors,
								  static_cast<VkPrimitiveTopology>(pipeline_settings.topology),
								  pipeline_settings.rasterizer_op,
								  pipeline_settings.depth_op,
								  attachment_blend_ops,
								  pipeline_settings.blend_constants,
								  &ctx.device().pipeline_cache().current_thread_cache());
	}

	// Creata the Vulkan framebuffer object
	void create_framebuffer() {
		if (!device_renderpass)
			create_renderpass();

		// Framebuffer's image views
		std::vector<VkImageView> image_view_handles;
		for (auto &a : fb) {
			// a is an optional attachment
			if (!a.second)
				continue;

			const framebuffer_attachment &attachment = a.second.get();
			image_view_handles.push_back(attachment.output->get_image_view_handle());
		}
		
		// Create Vulkan framebuffer
		device_framebuffer.emplace(ctx.device(),
								   device_renderpass.get(),
								   image_view_handles,
								   extent);
	}

	// Create the list of attachment clear values
	void create_clear_values() {
		std::vector<VkClearValue> vals;
		for (auto &a : fb) {
			// a is an optional attachment
			if (!a.second)
				continue;

			const framebuffer_attachment &attachment = a.second.get();
			const auto *image_ptr = attachment.output;
			VkClearValue val = image_ptr ?
				attachment.clear_value.vk_clear_value(image_ptr->get_format()) :
				VkClearValue{};

			vals.push_back(val);
		}

		clear_values = std::move(vals);
	}

protected:
	VkPipelineBindPoint get_pipeline_vk_bind_point() const override final {
		return VK_PIPELINE_BIND_POINT_GRAPHICS;
	}

	void bind_pipeline(const command_buffer &, command_recorder &recorder) const override final {
		recorder << cmd_begin_render_pass(device_framebuffer.get(),
										  device_renderpass.get(),
										  { 0,0 },
										  extent,
										  clear_values.get());
		recorder << cmd_bind_pipeline(graphics_pipeline.get());
	}
	
	void unbind_pipeline(const command_buffer &, command_recorder &recorder) const override final {
		recorder << cmd_end_render_pass();
	}

	void recreate_pipeline() override final {
		create_pipeline();
	}

	void update() override final {
		// Recreate invalidated objects before binding, as needed
		if (!device_framebuffer) {
			create_framebuffer();
		}
		if (!graphics_pipeline) {
			create_pipeline();
		}
		if (!clear_values) {
			create_clear_values();
		}
	}

public:
	device_pipeline_graphics(ctor,
							 const ste_context &ctx,
							 const device_pipeline_graphics_configurations &graphics_pipeline_settings,
							 const pipeline_vertex_input_bindings_collection::pipeline_vertex_input_bindings_descriptor &vertex_input_descriptor,
							 pipeline_binding_set_pool &pool,
							 pipeline_layout &&layout,
							 optional<std::reference_wrapper<const pipeline_external_binding_set_collection>> external_binding_sets)
		: Base(ctx,
			   pool,
			   std::move(layout),
			   external_binding_sets),
		pipeline_settings(graphics_pipeline_settings),
		vertex_input_descriptor(vertex_input_descriptor)
	{
		extent = {
			static_cast<std::uint32_t>(pipeline_settings.viewport.size.x),
			static_cast<std::uint32_t>(pipeline_settings.viewport.size.y)
		};
	}
	~device_pipeline_graphics() noexcept {}

	/**
	*	@brief	Returns a reference to the framebuffer object.
	*/
	auto framebuffer() {
		return fb_ref(this);
	}
};

}
}
