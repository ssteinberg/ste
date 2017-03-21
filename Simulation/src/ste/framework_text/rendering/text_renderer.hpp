// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_queue_selector.hpp>
#include <text_manager.hpp>
#include <attributed_string.hpp>
#include <glyph_point.hpp>

#include <vk_command_recorder.hpp>
#include <vk_cmd_begin_render_pass.hpp>
#include <vk_cmd_end_render_pass.hpp>
#include <vk_cmd_bind_pipeline.hpp>
#include <vk_cmd_bind_vertex_buffers.hpp>
#include <vk_cmd_bind_index_buffer.hpp>
#include <vk_cmd_draw.hpp>
#include <vk_cmd_bind_descriptor_sets.hpp>
#include <vk_cmd_pipeline_barrier.hpp>

#include <range.hpp>
#include <array.hpp>

namespace StE {
namespace Text {

class text_renderer {
	static constexpr std::size_t ringbuffer_max_size = 4096;

	using ring_buffer_type = GL::array<glyph_point>;

private:
	text_manager *tr;
	std::vector<glyph_point> points;

	ring_buffer_type vbo;

public:
	text_renderer(text_manager *tr);

	void set_text(const glm::vec2 &ortho_pos, const attributed_wstring &wstr);

	template <typename selector_policy = GL::ste_queue_selector_default_policy>
	void render(const GL::ste_queue_selector<selector_policy> &queue_selector) {
//		auto& q = tr->context.device().select_queue(queue_selector);
		auto batch = tr->context.device().allocate_presentation_command_batch(queue_selector);
		tr->context.device().enqueue(queue_selector, [this, batch = std::move(batch)]() mutable {
			auto& command_buffer = batch->acquire_command_buffer();
//			auto batch = GL::ste_device_queue::thread_allocate_batch();
//			auto& command_buffer = batch->acquire_command_buffer();
			{
				auto recorder = command_buffer.record();
				tr->update_glyphs(recorder);

				recorder
					<< vbo.update_cmd(points)
					<< GL::vk_cmd_pipeline_barrier(GL::vk_pipeline_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
																		   VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
																		   GL::vk_buffer_memory_barrier(vbo,
																										VK_ACCESS_TRANSFER_WRITE_BIT,
																										VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)))
					<< GL::vk_cmd_bind_descriptor_sets_graphics(tr->pipeline->pipeline_layout, 
																0, { tr->pipeline->descriptor_set.get() })
					<< GL::vk_cmd_begin_render_pass(tr->presentation_framebuffers[batch->presentation_image_index()],
													*tr->renderpass,
													{ 0,0 },
													{ 1920, 1080 },
													{ VkClearValue{} })
					<< GL::vk_cmd_bind_pipeline(tr->pipeline->pipeline)
					<< GL::vk_cmd_bind_vertex_buffers(0, vbo)
					<< GL::vk_cmd_draw(points.size(), 1, 0)
					<< GL::vk_cmd_end_render_pass();
			}

			tr->context.device().submit_and_present(std::move(batch));
		});
	}
};

}
}
