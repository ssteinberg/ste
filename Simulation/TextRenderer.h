// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "Font.h"
#include "glyph_manager.h"

#include "AttributedString.h"

#include "gl_current_context.h"
#include "StEngineControl.h"
#include "task.h"

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

class TextRenderer {
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
	};

	class text_renderable : public Graphics::renderable {
	private:
		TextRenderer *tr;
		std::vector<glyph_point> points;

		mutable range<> range_in_use;
		mutable int offset;
		mutable std::size_t elements_count, bytes;

	public:
		text_renderable(TextRenderer *tr, std::vector<glyph_point> &&points) : renderable(tr->text_distance_mapping), tr(tr), points(std::move(points)) {
			request_state({ GL_BLEND, true });
		}

		virtual void prepare() const override {
			renderable::prepare();

			LLR::gl_current_context::get()->blend_func(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			elements_count = std::min<std::size_t>(points.size(), vbo_ring_size / sizeof(glyph_point));
			bytes = elements_count * sizeof(glyph_point);
			if (tr->vbo_ring_current_offset + bytes > vbo_ring_size)
				tr->vbo_ring_current_offset = 0;

			offset = tr->vbo_ring_current_offset / sizeof(glyph_point);
			range_in_use = range<>(tr->vbo_ring_current_offset, bytes);
			tr->vbo_mapped_ptr.wait(range_in_use);
			memcpy(tr->vbo_mapped_ptr.get() + offset, &points[0], bytes);

			tr->text_distance_mapping->bind();

			tr->gm.ssbo().bind(LLR::shader_storage_layout_binding(0));
			tr->vao.bind();
		}

		virtual void render() const override {
			glDrawArrays(GL_POINTS, offset, elements_count);
		}

		virtual void finalize() const override {
			tr->vbo_mapped_ptr.lock(range_in_use);
			tr->vbo_ring_current_offset += bytes;
		}
	};

	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;

private:
	friend class text_renderable;

private:
	std::shared_ptr<LLR::GLSLProgram> text_distance_mapping;

	static constexpr int vbo_ring_size = 8192;
	static constexpr LLR::BufferUsage::buffer_usage buffer_usage = static_cast<LLR::BufferUsage::buffer_usage>(LLR::BufferUsage::BufferUsageMapCoherent | LLR::BufferUsage::BufferUsageMapWrite | LLR::BufferUsage::BufferUsageMapPersistent);
	using vbo_type = LLR::VertexBufferObject<glyph_point, glyph_point::descriptor, buffer_usage>;
	using vbo_mapped_ptr_type = LLR::mapped_buffer_object_unique_ptr<vbo_type::T, vbo_type::access_usage>;

	vbo_mapped_ptr_type vbo_mapped_ptr;
	std::unique_ptr<vbo_type> vbo;
	int vbo_ring_current_offset{ 0 };

	LLR::VertexArrayObject vao;

	glyph_manager gm;
	Font default_font;
	int default_size;

	void adjust_line(std::vector<glyph_point> &, const AttributedWString &, unsigned, float , float , const glm::vec2 &);
	std::vector<glyph_point> create_points(glm::vec2 , const AttributedWString &);

	std::shared_ptr<ResizeSignalConnectionType> resize_connection;

public:
	TextRenderer(const StEngineControl &context, const Font &default_font, int default_size = 28);

	std::unique_ptr<text_renderable> render(glm::vec2 ortho_pos, const AttributedWString &wstr);
};

}
}
