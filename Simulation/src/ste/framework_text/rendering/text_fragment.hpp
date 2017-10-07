//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <text_manager.hpp>
#include <attributed_string.hpp>
#include <glyph_point.hpp>
#include <fragment.hpp>

#include <command_recorder.hpp>
#include <task.hpp>
#include <cmd_draw.hpp>
#include <cmd_pipeline_barrier.hpp>

#include <stable_vector.hpp>
#include <alias.hpp>

namespace ste {
namespace text {

/**
*	@brief	Text glyph renderer fragment
*/
class text_fragment : public gl::fragment {
	using vector_type = gl::vector<glyph_point>;

private:
	alias<text_manager> tm;

	vector_type vertex_buffer;
	std::uint32_t count{ 0 };
	bool updated{ false };

	gl::task<gl::cmd_draw> draw_task;

private:
	class cmd_update_text : public gl::command {
		alias<const ste_context> ctx;
		text_fragment *tr;
		const lib::vector<glyph_point> points;

	public:
		cmd_update_text(const ste_context &ctx,
						text_fragment *tr,
						lib::vector<glyph_point> &&points)
			: ctx(ctx), tr(tr), points(std::move(points))
		{}
		virtual ~cmd_update_text() noexcept {}

	private:
		void operator()(const gl::command_buffer &, gl::command_recorder &recorder) && override final {
			// Update glyphs
			tr->tm->update_glyphs(recorder);
			tr->count = static_cast<std::uint32_t>(points.size());

			if (!tr->count)
				return;

			// Update vertex buffer
			if (tr->count > static_cast<std::uint32_t>(tr->vertex_buffer.size()))
				recorder << tr->vertex_buffer.resize_cmd(ctx.get(),
														 tr->count);
			recorder << tr->vertex_buffer.overwrite_cmd(0, points);

			tr->updated = true;
		}
	};

public:
	text_fragment(const ste_context &ctx,
				  text_manager*);

	text_fragment(text_fragment&&) = default;
	text_fragment &operator=(text_fragment&&) = default;

	/**
	*	@brief	Sets attributed text for the fragment to render. Will spawn tasks to load glyphs, if necessary.
	*/
	void update_text(const ste_context &ctx, 
					 gl::command_recorder &recorder,
					 const glm::vec2 &ortho_pos,
					 const attributed_wstring &wstr) {
		auto points = tm->create_points(ortho_pos, wstr);
		recorder << cmd_update_text(ctx, this, std::move(points));
	}

	/**
	*	@brief	Renders currently stored attributed text.
	*/
	void record(gl::command_recorder &recorder) override final {
		if (updated) {
			// Add pipeline barrier in case text was updated
			recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
																	  gl::pipeline_stage::vertex_input,
																	  gl::buffer_memory_barrier(*vertex_buffer,
																								gl::access_flags::transfer_write,
																								gl::access_flags::vertex_attribute_read)));
			recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
																	  gl::pipeline_stage::geometry_shader,
																	  gl::buffer_memory_barrier(tm->gm.ssbo(),
																								gl::access_flags::transfer_write,
																								gl::access_flags::shader_read)));
			updated = false;
		}

		if (count > 0)
			recorder << draw_task(count, 1);
	}

	auto& manager() { return *tm; }
	const auto& manager() const { return *tm; }

	static lib::string name() { return "text_renderer"; }
};

}
}
