
#include "stdafx.h"
#include "TextRenderer.h"

#include "gl_current_context.h"

#include "GLSLProgramLoader.h"

#include <vector>

#include <glm/glm.hpp>

using namespace StE::Text;
using namespace StE::LLR;

TextRenderer::TextRenderer(const StEngineControl &context, const Font &default_font, int default_size) : gm(context), context(context), default_font(default_font), default_size(default_size) {
	text_distance_mapping = Resource::GLSLProgramLoader::load_program_task(context, { "text_distance_map_contour.vert", "text_distance_map_contour.frag", "text_distance_map_contour.geom" })();

	vbo = std::make_unique<vbo_type>(vbo_ring_size);
	vbo_mapped_ptr = vbo->map_write(vbo_ring_size / sizeof(glyph_point), 0, static_cast<BufferUsage::buffer_mapping>(LLR::BufferUsage::BufferUsageMapCoherent | LLR::BufferUsage::BufferUsageMapPersistent));

	vao[0] = (*vbo)[0];
	vao[1] = (*vbo)[1];
	vao[2] = (*vbo)[2];
	vao[3] = (*vbo)[3];
	vao[4] = (*vbo)[4];
}

void TextRenderer::adjust_line(std::vector<glyph_point> &points, const AttributedWString &wstr, int line_start_index, float line_start, float line_height, const glm::vec2 &ortho_pos) {
	if (points.size() - line_start_index) {
		optional<const Attributes::align*> alignment_attrib = wstr.attrib_of_type(Attributes::align::attrib_type_s(), { line_start_index,points.size() - line_start_index });
		if (alignment_attrib && alignment_attrib->get() != Attributes::align::alignment::Left) {
			float line_len = ortho_pos.x - line_start;
			float offset = alignment_attrib->get() == Attributes::align::alignment::Center ? -line_len*.5f : -line_len;
			for (int i = line_start_index; i < points.size(); ++i)
				points[i].pos.x += offset;
		}
	}
	for (int i = line_start_index; i < points.size(); ++i)
		points[i].pos.y -= line_height;
}

std::vector<TextRenderer::glyph_point> TextRenderer::create_points(glm::vec2 ortho_pos, const AttributedWString &wstr) {
	float line_start = ortho_pos.x;
	int line_start_index = 0;
	float prev_line_height = 0;
	float line_height = 0;
	int num_lines = 1;

	std::vector<glyph_point> points;
	for (int i = 0; i < wstr.length(); ++i) {
		if (wstr[i] == '\n') {
			adjust_line(points, wstr, line_start_index, line_start, prev_line_height, ortho_pos);

			ortho_pos.x = line_start;
			ortho_pos.y -= prev_line_height;
			prev_line_height = line_height;
			line_height = 0;
			line_start_index = points.size();

			++num_lines;

			continue;
		}

		optional<const Attributes::font*> font_attrib = wstr.attrib_of_type(Attributes::font::attrib_type_s(), { i,1 });
		optional<const Attributes::rgb*> color_attrib = wstr.attrib_of_type(Attributes::rgb::attrib_type_s(), { i,1 });
		optional<const Attributes::size*> size_attrib = wstr.attrib_of_type(Attributes::size::attrib_type_s(), { i,1 });
		optional<const Attributes::stroke*> stroke_attrib = wstr.attrib_of_type(Attributes::stroke::attrib_type_s(), { i,1 });
		optional<const Attributes::weight*> weight_attrib = wstr.attrib_of_type(Attributes::weight::attrib_type_s(), { i,1 });

		Font &font = font_attrib ? font_attrib->get() : default_font;
		int size = size_attrib ? size_attrib->get() : default_size;
		glm::u8vec4 color = color_attrib ? color_attrib->get() : glm::u8vec4{255, 255, 255, 255};

		auto g = gm.glyph_for_font(font, wstr[i]);

		float f = static_cast<float>(size) / static_cast<float>(glyph::ttf_pixel_size);
		float w = weight_attrib ? weight_attrib->get() : 400.f;
		float lh = (g->metrics.height + g->metrics.start_y) * f * 2 + 1;

		float advance = i + 1 < wstr.length() ? gm.spacing(font, wstr[i], wstr[i + 1], size) : 0;

		glyph_point p;
		p.pos = ortho_pos.xy;
		p.glyph = g->buffer_index;
		p.size = f * 2;
		p.color = glm::vec4(color) / 255.0f;
		p.weight = glm::clamp<float>(w - 400, -300, 500) * f * .003f;

		if (stroke_attrib) {
			p.stroke_color = glm::vec4(stroke_attrib->get_color().get()) / 255.0f;
			p.stroke_width = stroke_attrib->get_width();
			advance += glm::floor(p.stroke_width * .5f);
		}
		else
			p.stroke_width = .0f;

		points.push_back(p);

		line_height = std::max(lh, line_height);
		ortho_pos.x += advance;
	}

	adjust_line(points, wstr, line_start_index, line_start, num_lines>1 ? line_height : 0, ortho_pos);

	return points;
}

void TextRenderer::render(glm::vec2 ortho_pos, const AttributedWString &wstr) {
	if (!wstr.length()) return;
	std::vector<glyph_point> points = create_points(ortho_pos, wstr);
	if (!points.size()) return;

	std::size_t elements_count= std::min<std::size_t>(points.size(), vbo_ring_size / sizeof(glyph_point));
	std::size_t bytes = elements_count * sizeof(glyph_point);
	if (vbo_ring_current_offset + bytes > vbo_ring_size)
		vbo_ring_current_offset = 0;

	int offset = vbo_ring_current_offset / sizeof(glyph_point);
	range<> range_in_use(vbo_ring_current_offset, bytes);
	vbo_mapped_ptr.wait(range_in_use);
	memcpy(vbo_mapped_ptr.get() + offset, &points[0], bytes);
	vbo_mapped_ptr.lock(range_in_use);

	text_distance_mapping->bind();
	text_distance_mapping->set_uniform("proj", context.ortho_projection_matrix());
	text_distance_mapping->set_uniform("fb_size", glm::vec2(context.get_backbuffer_size()));

	0_storage_idx = gm.ssbo();
	vao.bind();

	gl_current_context::get()->enable_state(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_POINTS, offset, elements_count);

	gl_current_context::get()->disable_state(GL_BLEND);

	vbo_ring_current_offset += bytes;
}
