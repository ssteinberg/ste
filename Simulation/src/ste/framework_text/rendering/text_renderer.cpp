
#include <stdafx.hpp>
#include <text_renderer.hpp>
#include <text_manager.hpp>

#include <attributed_string.hpp>

using namespace StE::Text;

text_renderer::text_renderer(text_manager *tr,
							 const StE::GL::vk_render_pass *renderpass)
	: tr(tr),
	fb_size_descriptor_set(tr->context.device(), { GL::vk_descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_GEOMETRY_BIT, 0) }),
	renderpass(renderpass),
	fb_size_uniform(tr->context, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
	vertex_buffer(tr->context, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) 
{
	fb_size_descriptor_set.get().write({
		GL::vk_descriptor_set_write_resource(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 0, GL::vk_descriptor_set_write_buffer(fb_size_uniform, 1))
	});
}

void text_renderer::recreate_pipeline() {
	glm::u32vec2 swapchain_size = tr->context.device().get_surface().size();
	VkViewport viewport = { 0, 0,
		static_cast<float>(swapchain_size.x), static_cast<float>(swapchain_size.y),
		1.f, 0.f };
	VkRect2D scissor = { { 0,0 },{ swapchain_size.x, swapchain_size.y } };

	this->pipeline_layout = std::make_unique<GL::vk_pipeline_layout>(GL::vk_pipeline_layout{
		tr->context.device(), { tr->descriptor_set->get_layout(), fb_size_descriptor_set.get_layout() }, {}
	});
	this->pipeline = std::make_unique<GL::vk_pipeline_graphics>(
		GL::vk_pipeline_graphics{
			tr->context.device(),
			{
				tr->vert->graphics_pipeline_stage_descriptor(),
				tr->geom->graphics_pipeline_stage_descriptor(),
				tr->frag->graphics_pipeline_stage_descriptor()
			},
			*pipeline_layout,
			*renderpass,
			0,
			viewport,
			scissor,
			{ { 0, glyph_point::descriptor() } },
			VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
			GL::vk_rasterizer_op_descriptor(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE),
			GL::vk_depth_op_descriptor(),
			{ GL::vk_blend_op_descriptor(VK_BLEND_FACTOR_SRC_ALPHA,
										 VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
										 VK_BLEND_OP_ADD) },
			glm::vec4{ .0f }
	});
}

