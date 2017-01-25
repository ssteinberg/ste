// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "glyph_point.hpp"

#include "gpu_dispatchable.hpp"
#include "gl_current_context.hpp"

#include "ring_buffer.hpp"
#include "range.hpp"

#include "vertex_array_object.hpp"
#include "vertex_buffer_object.hpp"
#include "shader_storage_buffer.hpp"

namespace StE {
namespace Text {

class text_manager;

class text_renderable : public Graphics::gpu_dispatchable {
	using Base = Graphics::gpu_task;

	static constexpr std::size_t ringbuffer_max_size = 4096;

	using ring_buffer_type = Core::ring_buffer<glyph_point, ringbuffer_max_size>;

private:
	mutable text_manager *tr;
	std::vector<glyph_point> points;

	ring_buffer_type vbo;
	Core::vertex_array_object vao;

	using vbo_type = Core::vertex_buffer_object<glyph_point, glyph_point::descriptor, decltype(vbo)::usage>;

	mutable range<> range_in_use{ 0, 0 };

public:
	text_renderable(text_manager *tr);

	void set_text(const glm::vec2 &ortho_pos, const attributed_wstring &wstr);

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
