
#include "stdafx.h"
#include "glyph_factory.h"

#include "make_distance_map.h"

#include "thread_constants.h"

#include <unordered_map>
#include <functional>

#include <mutex>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <exception>

using namespace StE::Text;

class glyph_factory_ft_lib {
private:
	FT_Library library;

public:
	glyph_factory_ft_lib() {
		auto error = FT_Init_FreeType(&library);
		if (error) {
			throw std::exception("Couldn't init FreeType");
		}
	}
	~glyph_factory_ft_lib() {
		FT_Done_FreeType(library);
	}
	glyph_factory_ft_lib(glyph_factory_ft_lib &&) = default;
	glyph_factory_ft_lib &operator=(glyph_factory_ft_lib &&) = default;

	FT_Library get_lib() { return library; }
};

class glyph_factory_font {
private:
	FT_Face face;

public:
	glyph_factory_font(const Font &font, FT_Library ft_lib) {
		auto error = FT_New_Face(ft_lib,
								 font.get_path().string().c_str(),
								 0,
								 &face);
		if (error) {
			throw std::exception("Failed loading FreeType font");
		}

		FT_Select_Charmap(face, ft_encoding_unicode);
	}
	~glyph_factory_font() {
		FT_Done_Face(face);
	}
	glyph_factory_font(glyph_factory_font &&) = default;
	glyph_factory_font &operator=(glyph_factory_font &&) = default;

	FT_Face get_face() { return face; }
};

struct StE::Text::glyph_factory_impl {
	std::mutex m;
	glyph_factory_ft_lib lib;
	std::unordered_map<Font, glyph_factory_font> fonts;

	auto get_face(const Font &font) {
		auto it = fonts.find(font);
		if (it == fonts.end())
			it = fonts.emplace(std::piecewise_construct,
							   std::forward_as_tuple(font),
							   std::forward_as_tuple(font, lib.get_lib())).first;
		return it->second.get_face();
	}

	unsigned char *render_glyph_with(const Font&, wchar_t, int, int&, int&, int&, int&);
};

unsigned char* glyph_factory_impl::render_glyph_with(const Font &font, wchar_t codepoint, int px_size, int &w, int &h, int &start_y, int &start_x) {
	std::unique_lock<std::mutex> l(m);

	auto face = get_face(font);

	FT_UInt glyph_index = FT_Get_Char_Index(face, codepoint);
	FT_Set_Pixel_Sizes(face, 0, px_size);
	FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
	FT_Render_Glyph(face->glyph, FT_RENDER_MODE_LIGHT);

	auto bm = face->glyph->bitmap;
	auto metrics = face->glyph->metrics;

	start_x = (metrics.horiBearingX >> 6) - glyph::padding;
	start_y = (metrics.horiBearingY - metrics.height >> 6) - glyph::padding;
	w = bm.width + 2 * glyph::padding;
	h = bm.rows + 2 * glyph::padding;

	int padding_w = std::max<int>(0, (w - bm.width) >> 1);
	int padding_h = std::max<int>(0, (h - bm.rows) >> 1);

	unsigned char *glyph_buf = new unsigned char[w*h];
	memset(glyph_buf, 0, w * h);
	for (int y = 0; y < bm.rows; ++y)
		memcpy(&glyph_buf[padding_w + (y + padding_h) * w], &reinterpret_cast<char*>(bm.buffer)[(bm.rows - y - 1) * bm.width], std::min<int>(w, bm.width));

	return glyph_buf;
}

glyph_factory::glyph_factory() : pimpl(std::make_unique<glyph_factory_impl>()) {}

glyph_factory::~glyph_factory() {}

StE::task<glyph> glyph_factory::create_glyph_task(const Font &font, wchar_t codepoint) {
	return task<glyph>([=](optional<task_scheduler*> sched) -> glyph {
		glyph g;
		int start_x, start_y, w, h;
		int px_size = glyph::ttf_pixel_size;

		unsigned char *glyph_buf = pimpl->render_glyph_with(font, codepoint, px_size, w, h, start_y, start_x);
		g.metrics.start_x = start_x;
		g.metrics.start_y = start_y;
		g.metrics.width = w;
		g.metrics.height = h;

		gli::texture2D text_glyph_distance_field_image(1, gli::format::FORMAT_R32_SFLOAT, { w,h });
		make_distance_map(glyph_buf, w, h, reinterpret_cast<float*>(text_glyph_distance_field_image[0].data()));
		delete[] glyph_buf;

		g.glyph_distance_field = std::move(text_glyph_distance_field_image);

		return std::move(g);
	});
}

int glyph_factory::spacing(const Font &font, wchar_t left, wchar_t right, int pixel_size) {
	std::unique_lock<std::mutex> l(pimpl->m);

	auto face = pimpl->get_face(font);

	FT_UInt left_index = FT_Get_Char_Index(face, left);
	FT_UInt right_index = FT_Get_Char_Index(face, right);
	FT_Set_Pixel_Sizes(face, 0, pixel_size);
	FT_Load_Glyph(face, left_index, FT_LOAD_DEFAULT);

	bool has_kern = FT_HAS_KERNING(face);

	if (has_kern) {
		FT_Vector delta;
		FT_Get_Kerning(face, left_index, right_index, FT_KERNING_DEFAULT, &delta);

		return ((face->glyph->advance.x + delta.x) >> 6) - 2;
	}
	else {
		return (face->glyph->advance.x >> 6) - 2;
	}
}
