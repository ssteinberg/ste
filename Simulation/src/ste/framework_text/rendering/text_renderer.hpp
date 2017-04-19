// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <text_manager.hpp>
#include <attributed_string.hpp>
#include <glyph_point.hpp>

#include <command_recorder.hpp>
#include <cmd_bind_pipeline.hpp>
#include <cmd_bind_vertex_buffers.hpp>
#include <cmd_draw.hpp>
#include <cmd_bind_descriptor_sets.hpp>
#include <cmd_pipeline_barrier.hpp>

#include <vk_render_pass.hpp>

#include <stable_vector.hpp>
#include <array.hpp>

namespace StE {
namespace Text {

class text_renderer {
	using vector_type = GL::stable_vector<glyph_point>;

private:
	text_manager *tr;

	GL::vk_unique_descriptor_set fb_size_descriptor_set;
	std::unique_ptr<GL::vk_pipeline_layout> pipeline_layout;
	std::unique_ptr<GL::vk_pipeline_graphics> pipeline;
	const GL::vk_render_pass *renderpass;
	GL::array<glm::vec2> fb_size_uniform;
	vector_type vertex_buffer;

	std::uint32_t count{ 0 };

private:
	class cmd_update_text : public GL::command {
		text_renderer *tr;
		const std::vector<glyph_point> points;
		const glm::vec2 render_target_size;

	public:
		cmd_update_text(text_renderer *tr,
						std::vector<glyph_point> &&points,
						const glm::vec2 &render_target_size)
			: tr(tr), points(std::move(points)), render_target_size(render_target_size)
		{}
		virtual ~cmd_update_text() noexcept {}

	private:
		void operator()(const GL::command_buffer &, GL::command_recorder &recorder) const override final {
			if (tr->tr->update_glyphs(recorder)) {
				// Recreate pipeline
				tr->recreate_pipeline();
			}

			recorder
				<< tr->vertex_buffer.resize_cmd(points.size())
				<< tr->vertex_buffer.update_cmd(points, 0);
			auto buffer_barrier = GL::buffer_memory_barrier(*tr->vertex_buffer,
															VK_ACCESS_TRANSFER_WRITE_BIT,
															VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
			recorder << GL::cmd_pipeline_barrier(GL::pipeline_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
																	  VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
																	  buffer_barrier));

			recorder << tr->fb_size_uniform.update_cmd({ render_target_size });
			recorder << GL::cmd_pipeline_barrier(GL::pipeline_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
																	  VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
																	  GL::buffer_memory_barrier(*tr->fb_size_uniform,
																								VK_ACCESS_TRANSFER_WRITE_BIT,
																								VK_ACCESS_UNIFORM_READ_BIT)));

			tr->count = points.size();
		}
	};

	class cmd_text_render : public GL::command {
		text_renderer *tr;

	public:
		cmd_text_render(text_renderer *tr)
			: tr(tr)
		{}
		virtual ~cmd_text_render() noexcept {}

	private:
		void operator()(const GL::command_buffer &, GL::command_recorder &recorder) const override final {
			if (!tr->count) {
				return;
			}

			recorder << GL::cmd_bind_descriptor_sets_graphics(*tr->pipeline_layout,
															  0, { &tr->tr->descriptor_set->get(), &tr->fb_size_descriptor_set.get() });
			recorder << GL::cmd_bind_pipeline(*tr->pipeline);
			recorder << GL::cmd_bind_vertex_buffers(0, tr->vertex_buffer);
			recorder << GL::cmd_draw(tr->count, 1, 0);
		}
	};

private:
	void recreate_pipeline();

public:
	text_renderer(text_manager *tr,
				  const GL::vk_render_pass *renderpass);

	auto update_cmd(const glm::vec2 &ortho_pos, 
					const attributed_wstring &wstr,
					const glm::vec2 &render_target_size) {
		 auto points = tr->create_points(ortho_pos, wstr);
		 return cmd_update_text(this, std::move(points), render_target_size);
	}
	auto render_cmd() {
		return cmd_text_render(this);
	}
};

}
}
