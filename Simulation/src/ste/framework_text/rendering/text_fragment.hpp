// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <text_manager.hpp>
#include <attributed_string.hpp>
#include <glyph_point.hpp>
#include <fragment.hpp>

#include <command_recorder.hpp>
#include <task.hpp>
#include <cmd_draw.hpp>
#include <cmd_pipeline_barrier.hpp>

#include <stable_vector.hpp>

namespace ste {
namespace text {

class text_fragment : public gl::fragment {
	using vector_type = gl::stable_vector<glyph_point>;

private:
	text_manager *tm;

	vector_type vertex_buffer;
	std::uint32_t count{ 0 };
	bool updated{ false };

	gl::task<gl::cmd_draw> draw_task;

private:
	class cmd_update_text : public gl::command {
		text_fragment *tr;
		const std::vector<glyph_point> points;

	public:
		cmd_update_text(text_fragment *tr,
						std::vector<glyph_point> &&points)
			: tr(tr), points(std::move(points))
		{}
		virtual ~cmd_update_text() noexcept {}

	private:
		void operator()(const gl::command_buffer &, gl::command_recorder &recorder) const override final {
			// Update glyphs
			tr->tm->update_glyphs(recorder);
			tr->count = points.size();

			if (!tr->count)
				return;

			// Update vertex buffer
			if (tr->count > tr->vertex_buffer.size())
				recorder << tr->vertex_buffer.resize_cmd(tr->count);
//			recorder << tr->vertex_buffer.update_task(points, 0)();

			tr->updated = true;
		}
	};

public:
	text_fragment(text_manager*);

	text_fragment(text_fragment&&) = default;
	text_fragment &operator=(text_fragment&&) = default;

	void update_text(gl::command_recorder &recorder,
					 const glm::vec2 &ortho_pos,
					 const attributed_wstring &wstr) {
		auto points = tm->create_points(ortho_pos, wstr);
		recorder << cmd_update_text(this, std::move(points));
	}

	void record(gl::command_recorder &recorder) override final {
		if (updated) {
			// Add pipeline barrier in case text was updated
			recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
																	  gl::pipeline_stage::vertex_shader,
																	  gl::buffer_memory_barrier(*vertex_buffer,
																								gl::access_flags::transfer_write,
																								gl::access_flags::vertex_attribute_read)));
			updated = false;
		}

		if (count > 0)
			recorder << draw_task(count, 1);
	}

	auto& manager() { return *tm; }
	const auto& manager() const { return *tm; }

	static const std::string& name() { return "text_renderer"; }
};

}
}
