
#include "stdafx.hpp"
#include "text_renderable.hpp"
#include "TextManager.hpp"

#include "AttributedString.hpp"
#include "gl_current_context.hpp"

using namespace StE::Text;
using namespace StE::Core;

text_renderable::text_renderable(TextManager *tr) : tr(tr) {
	auto vbo_buffer = Core::buffer_object_cast<vbo_type>(vbo.get_buffer());
	vao[0] = vbo_buffer[0];
	vao[1] = vbo_buffer[1];
	vao[2] = vbo_buffer[2];
	vao[3] = vbo_buffer[3];
	vao[4] = vbo_buffer[4];
}

void text_renderable::set_text(const glm::vec2 &ortho_pos, const AttributedWString &wstr) {
	points = tr->create_points(ortho_pos, wstr);
	range_in_use = vbo.commit(points);
}

void text_renderable::set_context_state() const {
	GL::gl_current_context::get()->enable_state(GL::BasicStateName::BLEND);
	GL::gl_current_context::get()->blend_func(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	0_storage_idx = tr->gm.ssbo();
	vao.bind();

	tr->text_distance_mapping.get().bind();
}

void text_renderable::dispatch() const {
	int start = static_cast<int>(range_in_use.start / sizeof(glyph_point));
	if (start >=0 && points.size() > 0)
		Core::GL::gl_current_context::get()->draw_arrays(GL_POINTS, start, points.size());
}
