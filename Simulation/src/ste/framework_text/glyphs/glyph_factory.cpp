
#include <stdafx.hpp>
#include <glyph_factory.hpp>
#include <make_distance_map.hpp>
#include <text_font_key.hpp>
#include <text_glyph_pair_key.hpp>

#include <lib/flat_map.hpp>
#include <functional>

#include <mutex>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdexcept>

#include <lib/alloc.hpp>

using namespace ste::text;

namespace ste {
namespace text {

class glyph_factory_ft_lib {
private:
	FT_Library library;
	lib::flat_map<_internal::sized_glyph_pair_key, std::uint32_t> spacing_cache;

public:
	glyph_factory_ft_lib() {
		auto error = FT_Init_FreeType(&library);
		if (error) {
			throw std::runtime_error("Couldn't init FreeType");
		}
	}
	~glyph_factory_ft_lib() {
		FT_Done_FreeType(library);
	}
	glyph_factory_ft_lib(glyph_factory_ft_lib &&) = default;
	glyph_factory_ft_lib &operator=(glyph_factory_ft_lib &&) = default;

	FT_Library get_lib() { return library; }
	FT_Library get_lib() const { return library; }

	bool get_spacing(const font &font, wchar_t left, wchar_t right, std::uint32_t pixel_size, std::uint32_t *spacing) const {
		auto it = spacing_cache.find(_internal::sized_glyph_pair_key{ font, left, right, pixel_size });
		if (it != spacing_cache.end()) {
			*spacing = it->second;
			return true;
		}
		return false;
	}
	void insert_into_spacing_cache(const font &font, wchar_t left, wchar_t right, std::uint32_t pixel_size, std::uint32_t spacing) {
		spacing_cache[_internal::sized_glyph_pair_key{ font, left, right, pixel_size }] = spacing;
	}
};

class glyph_factory_font {
private:
	FT_Face face{ nullptr };

public:
	glyph_factory_font(const font &font, FT_Library ft_lib) {
		auto error = FT_New_Face(ft_lib,
								 font.get_path().string().c_str(),
								 0,
								 &face);
		if (error) {
			face = nullptr;
			throw std::runtime_error("Failed loading FreeType font");
		}

		FT_Select_Charmap(face, ft_encoding_unicode);
	}
	~glyph_factory_font() {
		if (face)
			FT_Done_Face(face);
	}
	glyph_factory_font(glyph_factory_font &&f) noexcept : face(f.face) {
		f.face = nullptr;
	}
	glyph_factory_font &operator=(glyph_factory_font &&f) noexcept {
		face = f.face;
		f.face = nullptr;

		return *this;
	}

	FT_Face get_face() { return face; }
	FT_Face get_face() const { return face; }
};

struct glyph_factory_impl {
	std::mutex m;
	glyph_factory_ft_lib lib;
	lib::flat_map<_internal::font_key, glyph_factory_font> fonts;

	auto& get_factory_font(const font &font) {
		auto it = fonts.lower_bound(font);
		if (it == fonts.end() || it->first != font)
			it = fonts.emplace_hint(it, std::make_pair(font, glyph_factory_font(font, lib.get_lib())));
		return it->second;
	}

	unsigned char *render_glyph_with(const font&, wchar_t, int, int&, int&, int&, int&);
};

}
}

unsigned char* glyph_factory_impl::render_glyph_with(const font &font, wchar_t codepoint, int px_size, int &w, int &h, int &start_y, int &start_x) {
	std::unique_lock<std::mutex> l(m);

	auto face = get_factory_font(font).get_face();

	FT_UInt glyph_index = FT_Get_Char_Index(face, codepoint);
	FT_Set_Pixel_Sizes(face, 0, px_size);
	FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
	FT_Render_Glyph(face->glyph, FT_RENDER_MODE_LIGHT);

	auto bm = face->glyph->bitmap;
	auto metrics = face->glyph->metrics;

	start_x = (metrics.horiBearingX >> 6) - glyph::padding;
	start_y = ((metrics.horiBearingY - metrics.height) >> 6) - glyph::padding;
	w = bm.width + 2 * glyph::padding;
	h = bm.rows + 2 * glyph::padding;

	int padding_w = std::max<int>(0, (w - bm.width) >> 1);
	int padding_h = std::max<int>(0, (h - bm.rows) >> 1);

	unsigned char *glyph_buf = lib::default_alloc<unsigned char[]>::make(w*h);
	memset(glyph_buf, 0, w * h);
	for (unsigned y = 0; y < bm.rows; ++y)
		memcpy(&glyph_buf[padding_w + (y + padding_h) * w], &reinterpret_cast<char*>(bm.buffer)[(bm.rows - y - 1) * bm.pitch], std::min<int>(w, bm.pitch));

	return glyph_buf;
}


glyph_factory::glyph_factory() : pimpl(lib::default_alloc<glyph_factory_impl>::make()) {}

glyph_factory::~glyph_factory() {
	lib::default_alloc<glyph_factory_impl>::destroy(pimpl);
}

glyph glyph_factory::create_glyph(const font &font, wchar_t codepoint) const {
	glyph g;
	int start_x = 0, start_y = 0, w = 0, h = 0;
	int px_size = glyph::ttf_pixel_size;

	unsigned char *glyph_buf = pimpl->render_glyph_with(font, codepoint, px_size, w, h, start_y, start_x);
	g.metrics.start_x = start_x;
	g.metrics.start_y = start_y;
	g.metrics.width = w;
	g.metrics.height = h;

	g.glyph_distance_field = lib::allocate_unique<glyph::glyph_distance_field_surface_t>(glm::u32vec2{ static_cast<std::uint32_t>(w), static_cast<std::uint32_t>(h) });
	make_distance_map(glyph_buf, w, h, reinterpret_cast<float*>(g.glyph_distance_field->data()));
	lib::default_alloc<unsigned char[]>::destroy(glyph_buf);

	return std::move(g);
}

std::uint32_t glyph_factory::read_kerning(const font &font, const std::pair<wchar_t, wchar_t> &p, std::uint32_t pixel_size) {
	std::unique_lock<std::mutex> l(pimpl->m);

	std::uint32_t spacing;
	if (pimpl->lib.get_spacing(font, p.first, p.second, pixel_size, &spacing))
		return spacing;

	glyph_factory_font &fac_font = pimpl->get_factory_font(font);
	auto face = fac_font.get_face();

	FT_UInt left_index = FT_Get_Char_Index(face, p.first);
	FT_UInt right_index = FT_Get_Char_Index(face, p.second);
	FT_Set_Pixel_Sizes(face, 0, pixel_size);
	FT_Load_Glyph(face, left_index, FT_LOAD_DEFAULT);

	const bool has_kern = FT_HAS_KERNING(face);

	if (has_kern) {
		FT_Vector delta;
		FT_Get_Kerning(face, left_index, right_index, FT_KERNING_DEFAULT, &delta);

		spacing = (face->glyph->advance.x + delta.x) >> 6;
	}
	else {
		spacing = face->glyph->advance.x >> 6;
	}

	pimpl->lib.insert_into_spacing_cache(font, p.first, p.second, pixel_size, spacing);

	return spacing;
}
