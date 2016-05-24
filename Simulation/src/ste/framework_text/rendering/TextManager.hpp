// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "Font.hpp"
#include "glyph_manager.hpp"

#include "AttributedString.hpp"

#include "gpu_dispatchable.hpp"
#include "gl_current_context.hpp"
#include "StEngineControl.hpp"
#include "task.hpp"

#include "ring_buffer.hpp"
#include "range.hpp"

#include "VertexArrayObject.hpp"
#include "VertexBufferObject.hpp"
#include "ShaderStorageBuffer.hpp"
#include "GLSLProgram.hpp"
#include "Sampler.hpp"

#include <memory>
#include <string>
#include <vector>

namespace StE {
namespace Text {

class TextManager {
private:
	struct glyph_point {
		glm::vec2 pos; float glyph, size;
		glm::vec4 color;
		glm::vec4 stroke_color;
		float stroke_width;
		float weight;

		using descriptor = Core::VBODescriptorWithTypes<glm::vec4, glm::vec4, glm::vec4, float, float>::descriptor;

		glyph_point() {}
	};

public:
	class text_renderable : public Graphics::gpu_dispatchable {
		using Base = Graphics::gpu_task;

		static constexpr std::size_t ringbuffer_max_size = 4096;

		using ring_buffer_type = Core::ring_buffer<glyph_point, ringbuffer_max_size>;

	private:
		mutable TextManager *tr;
		std::vector<glyph_point> points;

		ring_buffer_type vbo;
		Core::VertexArrayObject vao;

		using vbo_type = Core::VertexBufferObject<glyph_point, glyph_point::descriptor, decltype(vbo)::usage>;

		mutable range<> range_in_use{ 0, 0 };

	public:
		text_renderable(TextManager *tr);

		void set_text(const glm::vec2 &ortho_pos, const AttributedWString &wstr) {
			points = tr->create_points(ortho_pos, wstr);
			range_in_use = vbo.commit(points);
		}

	protected:
		void set_context_state() const override final;
		void dispatch() const override final;
	};

	using ResizeSignalConnectionType = StEngineControl::framebuffer_resize_signal_type::connection_type;

private:
	friend class text_renderable;

private:
	std::shared_ptr<Core::GLSLProgram> text_distance_mapping;
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;

	glyph_manager gm;
	Font default_font;
	int default_size;

private:
	void adjust_line(std::vector<glyph_point> &, const AttributedWString &, unsigned, float , float , const glm::vec2 &);
	std::vector<glyph_point> create_points(glm::vec2, const AttributedWString &);

public:
	TextManager(const StEngineControl &context, const Font &default_font, int default_size = 28);

	std::unique_ptr<text_renderable> create_renderer() {
		return std::make_unique<text_renderable>(this);
	}
};

}
}
