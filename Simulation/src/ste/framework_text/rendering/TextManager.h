// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "Font.h"
#include "glyph_manager.h"

#include "AttributedString.h"

#include "gpu_task.h"
#include "gl_current_context.h"
#include "StEngineControl.h"
#include "task.h"

#include "ring_buffer.h"

#include "VertexArrayObject.h"
#include "VertexBufferObject.h"
#include "ShaderStorageBuffer.h"
#include "GLSLProgram.h"
#include "Sampler.h"

#include <memory>
#include <string>
#include <vector>

namespace StE {
namespace Text {

class TextManager {
private:
	struct glyph_point {
		struct {
			glm::vec2 pos;
			float glyph, size;
		};
		glm::vec4 color;
		glm::vec4 stroke_color;
		float stroke_width;
		float weight;

		using descriptor = LLR::VBODescriptorWithTypes<glm::vec4, glm::vec4, glm::vec4, float, float>::descriptor;
		
		glyph_point() {}
	};

	class text_renderable : public gpu_task {
	private:
		mutable TextManager *tr;
		std::vector<glyph_point> points;

		mutable range<> range_in_use;

	public:
		text_renderable(TextManager *tr) : tr(tr) {}
		
		void set_text(const glm::vec2 &ortho_pos, const AttributedWString &wstr) {
			points = tr->create_points(ortho_pos, wstr);
		}
		
		void set_context_state() const override final {
			using namespace LLR;
			
			LLR::gl_current_context::get()->enable_state(BLEND);
			LLR::gl_current_context::get()->blend_func(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			tr->text_distance_mapping->bind();
			0_storage_idx = tr->gm.ssbo();
			tr->vao.bind();
		}

		void dispatch() const override final {	
			range_in_use = tr->vbo.commit(points);
					
			LLR::gl_current_context::get()->draw_arrays(GL_POINTS, range_in_use.start / sizeof(glyph_point), points.size());
			
			tr->vbo.lock_range(range_in_use);
		}
	};

	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;

private:
	friend class text_renderable;

private:
	std::shared_ptr<LLR::GLSLProgram> text_distance_mapping;
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;

	static constexpr int vbo_ring_max_size = 4096;

	LLR::ring_buffer<glyph_point, vbo_ring_max_size> vbo;
	LLR::VertexArrayObject vao;

	using vbo_type = LLR::VertexBufferObject<glyph_point, glyph_point::descriptor, decltype(vbo)::usage>;

	glyph_manager gm;
	Font default_font;
	int default_size;

private:
	void adjust_line(std::vector<glyph_point> &, const AttributedWString &, unsigned, float , float , const glm::vec2 &);
	std::vector<glyph_point> create_points(const glm::vec2 &, const AttributedWString &);

public:
	TextManager(const StEngineControl &context, const Font &default_font, int default_size = 28);

	std::unique_ptr<text_renderable> create_render() {
		return std::make_unique<text_renderable>(this);
	}
};

}
}
