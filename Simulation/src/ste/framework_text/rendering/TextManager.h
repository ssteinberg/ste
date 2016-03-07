// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "Font.h"
#include "glyph_manager.h"

#include "AttributedString.h"

#include "gl_current_context.h"
#include "StEngineControl.h"
#include "task.h"

#include "ring_buffer.h"

#include "renderable.h"

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

	class text_renderable : public Graphics::renderable {
	private:
		TextManager *tr;
		std::vector<glyph_point> points;

		mutable range<> range_in_use;

	public:
		text_renderable(TextManager *tr, std::vector<glyph_point> &&points) : renderable(tr->text_distance_mapping), tr(tr), points(std::move(points)) {
			request_state({ GL_BLEND, true });
		}

		virtual void prepare() const override {
			renderable::prepare();

			LLR::gl_current_context::get()->blend_func(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			tr->gm.ssbo().bind(LLR::shader_storage_layout_binding(0));
			tr->vao.bind();

			range_in_use = tr->vbo.commit(points);
		}

		virtual void render() const override {
			glDrawArrays(GL_POINTS, range_in_use.start / sizeof(glyph_point), points.size());
		}

		virtual void finalize() const override {
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
	std::vector<glyph_point> create_points(glm::vec2 , const AttributedWString &);

public:
	TextManager(const StEngineControl &context, const Font &default_font, int default_size = 28);

	std::unique_ptr<text_renderable> render(glm::vec2 ortho_pos, const AttributedWString &wstr);
};

}
}
