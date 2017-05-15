//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_pipeline.hpp>
#include <device_pipeline_exceptions.hpp>
#include <device_pipeline_graphics_configurations.hpp>

#include <framebuffer_layout.hpp>
#include <framebuffer.hpp>

#include <pipeline_vertex_input_bindings_collection.hpp>
#include <pipeline_dynamic_state.hpp>
#include <vk_render_pass.hpp>
#include <vk_pipeline_graphics.hpp>

#include <cmd_begin_render_pass.hpp>
#include <cmd_end_render_pass.hpp>
#include <cmd_bind_pipeline.hpp>
#include <cmd_set_viewport.hpp>
#include <cmd_set_scissor.hpp>

#include <optional.hpp>
#include <format_rtti.hpp>
#include <vector>

namespace ste {
namespace gl {

class device_pipeline_graphics : public device_pipeline {
	using Base = device_pipeline;

	friend class pipeline_auditor_graphics;

	struct ctor {};

private:
	device_pipeline_graphics_configurations pipeline_settings;
	pipeline_vertex_input_bindings_collection::pipeline_vertex_input_bindings_descriptor vertex_input_descriptor;
	framebuffer_layout fb_layout;

	optional<vk::vk_render_pass> device_renderpass;
	framebuffer* attached_framebuffer{ nullptr };
	optional<vk::vk_pipeline_graphics> graphics_pipeline;

private:
	void invalidate_pipeline() {
		graphics_pipeline = none;
	}

private:
	// Creates the Vulkan renderpass object
	void create_renderpass() {
		auto renderpass = fb_layout.create_compatible_renderpass(ctx.get());
		device_renderpass.emplace(std::move(renderpass));
	}

	// Creates the graphics pipeline object
	void create_pipeline() {
		// Vulkan shader stage descriptors
		auto shader_stage_descriptors = get_layout().shader_stage_descriptors();

		// Blend operation descriptor for each attachment
		std::vector<vk::vk_blend_op_descriptor> attachment_blend_ops;
		for (auto &a : fb_layout) {
			const framebuffer_attachment_layout &attachment = a.second;

			// Blend operation is only applicable for color attachments
			bool is_depth_attachment = format_is_depth(attachment.image_format);
			if (is_depth_attachment)
				continue;

			vk::vk_blend_op_descriptor desc = attachment.blend;
			attachment_blend_ops.push_back(std::move(desc));
		}

		// Set viewport and scissor as dynamic states
		std::vector<VkDynamicState> dynamic_states = {
			static_cast<VkDynamicState>(pipeline_dynamic_state::viewport),
			static_cast<VkDynamicState>(pipeline_dynamic_state::scissor),
		};

		// Create the graphics pipeline object
		graphics_pipeline.emplace(ctx.get().device(),
								  shader_stage_descriptors,
								  get_layout(),
								  device_renderpass.get(),
								  0,
								  VkViewport{},
								  VkRect2D{},
								  vertex_input_descriptor.vertex_input_binding_descriptors,
								  vertex_input_descriptor.vertex_input_attribute_descriptors,
								  static_cast<VkPrimitiveTopology>(pipeline_settings.topology),
								  pipeline_settings.rasterizer_op,
								  pipeline_settings.depth_op,
								  attachment_blend_ops,
								  pipeline_settings.blend_constants,
								  dynamic_states,
								  &ctx.get().device().pipeline_cache().current_thread_cache());
	}

protected:
	VkPipelineBindPoint get_pipeline_vk_bind_point() const override final {
		return VK_PIPELINE_BIND_POINT_GRAPHICS;
	}

	void bind_pipeline(const command_buffer &, command_recorder &recorder) const override final {
		std::vector<VkClearValue> clear_values(attached_framebuffer->get_fb_clearvalues().begin(),
											   attached_framebuffer->get_fb_clearvalues().begin() + fb_layout.get_highest_index_of_attachment_with_load_op());
		auto fb_extent = attached_framebuffer->extent();

		recorder << cmd_begin_render_pass(*attached_framebuffer,
										  device_renderpass.get(),
										  { 0,0 },
										  fb_extent,
										  clear_values);
		recorder << cmd_bind_pipeline(graphics_pipeline.get());

		// Set dynamic viewport and scissor states
		recorder
			<< cmd_set_viewport(rect(glm::vec2(fb_extent)), attached_framebuffer->get_depth_range())
			<< cmd_set_scissor(i32rect(glm::i32vec2(fb_extent)));
	}
	
	void unbind_pipeline(const command_buffer &, command_recorder &recorder) const override final {
		recorder << cmd_end_render_pass();
	}

	optional<vk::vk_pipeline> recreate_pipeline() override final {
		// Slice old pipeline, if any, storing the old vk::vk_pipeline object.
		optional<vk::vk_pipeline> old_pipeline;
		if (graphics_pipeline) {
			vk::vk_pipeline &old_pipeline_object = graphics_pipeline.get();
			old_pipeline = std::move(old_pipeline_object);
		}

		create_pipeline();

		return old_pipeline;
	}

	void update() override final {
		// Make sure we have framebuffer attached
		if (attached_framebuffer == nullptr) {
			throw device_pipeline_no_attached_framebuffer_exception("Attempting to bind graphics pipeline without attaching a framebuffer first");
		}
		// Update framebuffer
		attached_framebuffer->update();

		// Recreate invalidated pipeline object before binding, as needed
		if (!graphics_pipeline) {
			create_pipeline();
		}
	}

public:
	device_pipeline_graphics(ctor,
							 const ste_context &ctx,
							 const device_pipeline_graphics_configurations &graphics_pipeline_settings,
							 const pipeline_vertex_input_bindings_collection::pipeline_vertex_input_bindings_descriptor &vertex_input_descriptor,
							 const framebuffer_layout &fb_layout,
							 pipeline_layout &&layout,
							 optional<std::reference_wrapper<const pipeline_external_binding_set_collection>> external_binding_sets)
		: Base(ctx,
			   std::move(layout),
			   external_binding_sets),
		pipeline_settings(graphics_pipeline_settings),
		vertex_input_descriptor(vertex_input_descriptor),
		fb_layout(fb_layout)
	{
		// Create the renderpass based on the framebuffer layout.
		// The renderpass object does not get invalidated.
		create_renderpass();
	}
	~device_pipeline_graphics() noexcept {}

	device_pipeline_graphics(device_pipeline_graphics&&) = default;
	device_pipeline_graphics &operator=(device_pipeline_graphics&&) = default;

	/**
	*	@brief	Attaches a framebuffer object.
	*			The new framebuffer object replaces any previously attached framebuffer objects, if any.
	*/
	auto attach_framebuffer(framebuffer &fb) {
		// Validate
		if (!fb_layout.compatible(fb.get_layout())) {
			throw device_pipeline_incompatible_framebuffer_exception("Attempting to attach a framebuffer object with incompatible layout");
		}

		return this->attached_framebuffer = &fb;
	}

	/**
	*	@brief	Returns the pipeline's expected framebuffer layout
	*/
	const auto& get_framebuffer_layout() const {
		return fb_layout;
	}
};

}
}
