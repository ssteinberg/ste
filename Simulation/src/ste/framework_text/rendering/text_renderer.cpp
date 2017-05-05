
#include <stdafx.hpp>
#include <text_renderer.hpp>
#include <text_manager.hpp>

#include <attributed_string.hpp>

using namespace ste::text;

text_renderer::text_renderer(text_manager *tr,
							 const ste::gl::vk::vk_render_pass *renderpass)
	: tr(tr),
	fb_size_descriptor_set(tr->context.device(), { gl::vk::vk_descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_GEOMETRY_BIT, 0) }),
	renderpass(renderpass),
	fb_size_uniform(tr->context, 1, gl::buffer_usage::uniform_buffer),
	vertex_buffer(tr->context, gl::buffer_usage::vertex_buffer) 
{
//	fb_size_descriptor_set.get().write({
//		gl::vk_descriptor_set_write_resource(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 0, gl::vk_descriptor_set_write_buffer(fb_size_uniform, 1))
//	});
}

void text_renderer::recreate_pipeline() {
	glm::u32vec2 swapchain_size = tr->context.device().get_surface().extent();
	VkViewport viewport = { 0, 0,
		static_cast<float>(swapchain_size.x), static_cast<float>(swapchain_size.y),
		1.f, 0.f };
	VkRect2D scissor = { { 0,0 },{ swapchain_size.x, swapchain_size.y } };

	this->pipeline_layout = std::make_unique<gl::vk::vk_pipeline_layout>(gl::vk::vk_pipeline_layout{
		tr->context.device(), { &tr->descriptor_set->get_layout(), &fb_size_descriptor_set.get_layout() }, {}
	});
//	this->pipeline = std::make_unique<gl::vk_pipeline_graphics>(
//		gl::vk_pipeline_graphics{
//			tr->context.device(),
//			{
//				tr->vert->pipeline_stage_descriptor(),
//				tr->geom->pipeline_stage_descriptor(),
//				tr->frag->pipeline_stage_descriptor()
//			},
//			*pipeline_layout,
//			*renderpass,
//			0,
//			viewport,
//			scissor,
//			{ { 0, glyph_point::descriptor() } },
//			VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
//			gl::vk_rasterizer_op_descriptor(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE),
//			gl::vk_depth_op_descriptor(),
//			{ gl::vk_blend_op_descriptor(VK_BLEND_FACTOR_SRC_ALPHA,
//										 VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
//										 VK_BLEND_OP_ADD) },
//			glm::vec4{ .0f },
//			&tr->context.device().pipeline_cache().current_thread_cache()
//	});
}

