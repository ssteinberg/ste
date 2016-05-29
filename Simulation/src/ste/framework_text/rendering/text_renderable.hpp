// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "glyph_point.hpp"

#include "gpu_dispatchable.hpp"
#include "gl_current_context.hpp"

#include "ring_buffer.hpp"
#include "range.hpp"

#include "VertexArrayObject.hpp"
#include "VertexBufferObject.hpp"
#include "ShaderStorageBuffer.hpp"

namespace StE {
namespace Text {

class TextManager;

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

	void set_text(const glm::vec2 &ortho_pos, const AttributedWString &wstr);

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
