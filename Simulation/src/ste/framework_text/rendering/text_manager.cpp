
#include "stdafx.hpp"
#include "text_manager.hpp"

#include "gl_current_context.hpp"

using namespace StE;
using namespace StE::Text;
using namespace StE::Core;

text_manager::text_manager(const ste_engine_control &context,
						 const font &default_font,
						 int default_size) : context(context),
						 					 gm(context),
											 default_font(default_font),
											 default_size(default_size),
											 text_distance_mapping(context, std::vector<std::string>{ "text_distance_map_contour.vert", "text_distance_map_contour.frag", "text_distance_map_contour.geom" }) {
	resize_connection = std::make_shared<ResizeSignalConnectionType>([=](const glm::i32vec2 &size) {
		text_distance_mapping.get().set_uniform("proj", glm::ortho<float>(0, size.x, 0, size.y, -1, 1));
		text_distance_mapping.get().set_uniform("fb_size", glm::vec2(size));
	});
	context.signal_framebuffer_resize().connect(resize_connection);
}

void text_manager::adjust_line(std::vector<glyph_point> &points, const attributed_wstring &wstr, unsigned line_start_index, float line_start, float line_height, const glm::vec2 &ortho_pos) {
	if (points.size() - line_start_index) {
		auto alignment_attrib = Attributes::align::bind(wstr.attrib_of_type(Attributes::attrib_type::align, { line_start_index, static_cast<std::uint32_t>(points.size()) - line_start_index }));
		auto line_height_attrib = Attributes::line_height::bind(wstr.attrib_of_type(Attributes::attrib_type::line_height, { line_start_index, static_cast<std::uint32_t>(points.size()) - line_start_index }));

		if (alignment_attrib && alignment_attrib->get() != Attributes::align::alignment::Left) {
			float line_len = ortho_pos.x - line_start;
			float offset = alignment_attrib->get() == Attributes::align::alignment::Center ? -line_len*.5f : -line_len;
			for (unsigned i = line_start_index; i < points.size(); ++i)
				points[i].pos.x += offset;
		}

		if (line_height_attrib && line_height>0)
			line_height = line_height_attrib->get();
	}
	for (unsigned i = line_start_index; i < points.size(); ++i)
		points[i].pos.y -= line_height;
}

std::vector<glyph_point> text_manager::create_points(glm::vec2 ortho_pos, const attributed_wstring &wstr) {
	float line_start = ortho_pos.x;
	int line_start_index = 0;
	float prev_line_height = 0;
	float line_height = 0;
	int num_lines = 1;

	std::vector<glyph_point> points;
	for (unsigned i = 0; i < wstr.length(); ++i) {
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

		auto font_attrib = Attributes::font::bind(wstr.attrib_of_type(Attributes::attrib_type::font, { i,1 }));
		auto color_attrib = Attributes::rgb::bind(wstr.attrib_of_type(Attributes::attrib_type::color, { i,1 }));
		auto size_attrib = Attributes::size::bind(wstr.attrib_of_type(Attributes::attrib_type::size, { i,1 }));
		auto stroke_attrib = Attributes::stroke::bind(wstr.attrib_of_type(Attributes::attrib_type::stroke, { i,1 }));
		auto weight_attrib = Attributes::weight::bind(wstr.attrib_of_type(Attributes::attrib_type::weight, { i,1 }));

		const font &font = font_attrib ? font_attrib->get() : default_font;
		int size = size_attrib ? size_attrib->get() : default_size;
		glm::u8vec4 color = color_attrib ? color_attrib->get() : glm::u8vec4{255, 255, 255, 255};

		auto g = gm.glyph_for_font(&context.scheduler(), font, wstr[i]);

		float f = static_cast<float>(size) / static_cast<float>(glyph::ttf_pixel_size);
		float w = weight_attrib ? weight_attrib->get() : 400.f;
		float lh = (g->metrics.height + g->metrics.start_y) * f * 2 + 1;

		float advance = i + 1 < wstr.length() ? gm.spacing(font, { wstr[i], wstr[i + 1] }, size) : 0;

		glyph_point p;
		p.pos = decltype(p.pos){ ortho_pos.x, ortho_pos.y };
		p.glyph = g->buffer_index;
		p.size = f * 2;
		p.color = glm::vec4(color) / 255.0f;
		p.weight = glm::clamp<float>(w - 400, -300, 500) * f * .003f;

		if (stroke_attrib) {
			p.stroke_color = glm::vec4(stroke_attrib->get_color()) / 255.0f;
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
